// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use these files except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copyright 2012 - Gonzalo Iglesias, Adrià de Gispert, William Byrne

#ifndef TASK_SPWLATS_HPP
#define TASK_SPWLATS_HPP

/**
 * \file
 * \brief Implements the task of creating sparse vector weight lattices -- contains feature weight contributions separately in each arc
 * and we can use it to dump features, MERT training, etc
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Creates lattices using tropical tuple weight semiring -- each arc containing separate feature weight contributions.
 * Note that the semiring is tropical under dot product of all these features with its scales
 */
template<class Data, class Arc>
class SparseWeightVectorLatticesTask: public ucam::util::TaskInterface<Data> {
 private:
  //key for a pointer in the data object to read the alignment lattice
  const std::string alilatskey_;
  ///key for a pointer in the data object to store the sparse weight vector lattice
  const std::string sparseweightvectorlatticekey_;
  ///key for a pointer to language model(s).
  const std::string lmkey_;
  ///key for a pointer to the rule flower lattice
  const std::string ruleflowerlatticekey_;

  ///Sparse tuple weight lattice
  fst::VectorFst<TupleArc32> myfst_;

  ///Pointer to data object
  Data *d_;

  //Strip dr, oov, ...
  bool stripHifstEpsilons_;
  fst::RelabelUtil<TupleArc32> ru_;
 public:
  ///Constructor with registry object and keys to access/write lattices in data object
  SparseWeightVectorLatticesTask ( const ucam::util::RegistryPO& rg,
                                   const std::string& alilatskey =
                                     HifstConstants::kSparseweightvectorlatticeLoadalilats,
                                   const std::string& ruleflowerlatticekey =
                                     HifstConstants::kRuleflowerlatticeStore,
                                   const std::string& sparseweightvectorlatticekey =
                                     HifstConstants::kSparseweightvectorlatticeStore
                                 ) :
    ruleflowerlatticekey_ ( ruleflowerlatticekey )
    , alilatskey_ ( alilatskey )
    , sparseweightvectorlatticekey_ ( sparseweightvectorlatticekey )
    , stripHifstEpsilons_ (rg.getBool (
                             HifstConstants::kSparseweightvectorlatticeStripSpecialEpsilonLabels) ) {
    ru_.addIPL (DR, EPSILON)
    .addIPL (OOV, EPSILON)
    .addIPL (SEP, EPSILON)
    ;
  };

  /**
   * \brief Implements virtual method from ucam::util::TaskInterface.
   * \remark Takes an alignment lattice, maps to tuplearc32 and composes it with the grammar flower lattice.
   * After projecting on the words (thus deleting rules), we have a word lattice
   * containing independent feature contributions to weight in each arc.
   * \param d    Data object
   */
  bool run ( Data& d ) {
    myfst_.DeleteStates();
    d_ = &d;
    fst::VectorFst<Arc> *lattice = static_cast<fst::VectorFst<Arc> *>
                                   ( d.fsts[alilatskey_] );
    USER_CHECK ( lattice->NumStates(), "Empty alignment lattice?" );
    Invert ( lattice );
    fst::VectorFst<TupleArc32> *lxr
      = applyFlowerLattice (*lattice
                            , * ( static_cast<fst::VectorFst<TupleArc32> *>
                                  ( d.fsts[ruleflowerlatticekey_] ) ) );
    LDBG_EXECUTE ( lattice->Write ( "fsts/aplats/aplats+flower.fst" ) );
    if (stripHifstEpsilons_) {
      LINFO ("Remove hifst epsilons");
      ru_ (lxr);
    }
    LINFO ( "Project, RmEpsilon, Determinize and Minimize" );
    fst::Project ( lxr, fst::PROJECT_INPUT );
    LDBG_EXECUTE ( lxr->Write ( "fsts/aplats/aplats+flower+p.fst.gz" ) );
    fst::RmEpsilon<TupleArc32> ( lxr );
    fst::Determinize<TupleArc32> ( *lxr, &myfst_ );
    LDBG_EXECUTE ( myfst_.Write ( "fsts/aplats/aplats+flower+p+re+d.fst" ) );
    delete lxr;
    fst::Minimize<TupleArc32> ( &myfst_ );
    LDBG_EXECUTE ( myfst_.Write ( "fsts/aplats/aplats+flower+p+re+d+m.fst" ) );
    d.fsts[sparseweightvectorlatticekey_] = &myfst_;
    LINFO ( "Ready! Stored with key=" << sparseweightvectorlatticekey_ << "; NS=" <<
            myfst_.NumStates() );
    return false;
  };

  ~SparseWeightVectorLatticesTask() {
    myfst_.DeleteStates();
    LINFO ("Shutdown!");
  }

 private:

  ///Takes care of composition with the grammar flower lattice. Alignment lattice must be mapped first to  tropical sparse tuple weight semiring.
  fst::VectorFst<TupleArc32> * applyFlowerLattice ( const fst::VectorFst<Arc>
      & hypfst,
      const fst::VectorFst<TupleArc32>& grammarflowerlattice ) {
    LINFO ( "Removing Weights of Target Lattice" );
    fst::VectorFst<Arc> prvfst ( hypfst );
    fst::Map ( &prvfst, fst::RmWeightMapper<Arc>() );
    fst::VectorFst<TupleArc32> *vwfst = new fst::VectorFst<TupleArc32>;
    LINFO ( "Mapping Arc Target Lattice to TupleArc32" );
    fst::MakeSparseVectorWeight<Arc> mwcopy ( 1 );
    fst::Map<Arc> ( prvfst, vwfst,
                    fst::GenericWeightMapper<Arc, TupleArc32, fst::MakeSparseVectorWeight<Arc>  > ( mwcopy ) );
    fst::VectorFst<TupleArc32> *output = new fst::VectorFst<TupleArc32>;
    LDBG_EXECUTE ( vwfst->Write ( "fsts/aplats/vw-beforecomposing.fst" ) );
    LINFO ( "Compose (TupleArc32)" );
    fst::Compose ( *vwfst, grammarflowerlattice, output );
    delete vwfst;
    return output;
  };

  ZDISALLOW_COPY_AND_ASSIGN ( SparseWeightVectorLatticesTask );

};

}
} // end namespaces

#endif // TASK_ALATS2SPWLATS_HPP
