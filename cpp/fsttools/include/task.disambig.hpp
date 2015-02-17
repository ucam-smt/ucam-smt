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

#ifndef TASK_DISAMBIG_HPP
#define TASK_DISAMBIG_HPP

/**
 * \file
 * \brief Implemention of class DisambigTask
 * \date October 2012
 * \author Gonzalo Iglesias
 */

#include "task.applylm.kenlmtype.hpp"
#include "task.disambig.flowerfst.hpp"

namespace ucam {
namespace fsttools {

/**
 * \brief Disambig Task tool.
 * Given a search space, applies a unigram transduction model (generating alternatives) and an ngram model over alternatives
 */
template<class Data, class Arc>
class DisambigTask : public ucam::util::TaskInterface<Data> {
private:
  typedef typename Arc::Weight Weight;
  typedef typename Arc::Label Label;

  ///Fst that maps e.g. lower case unigrams (unimap from now on) to upper case versions as seen in training data
  fst::VectorFst<Arc> *unimap_;
  ///Shortest path value
  unsigned shp_;

  ///Pruning weight
  float prune_;

  ///ucam::util::RegistryPO object with program options
  const ucam::util::RegistryPO& rg_;

  ///Language model key
  const std::string lmkey_;
  ///Lattice Input key
  const std::string inputkey_;
  ///Lattice output key
  const std::string outputkey_;
  ///Unimap key
  const std::string unimapkey_;

  ///Pointer to current data object
  Data *d_;

  ///MakeWeight functor for transparent weight management (StdArc,LexStdArc)
  fst::MakeWeight2<Arc> mw_;

  ///Output lattice
  fst::VectorFst<Arc> olattice_;

 public:
  ///Constructor with registry objects and several keys to access either ucam::util::RegistryPO program options or the data object itself
  DisambigTask ( const ucam::util::RegistryPO& rg,
                 const std::string& inputkey = HifstConstants::kRecaserInput,
                 const std::string& outputkey = HifstConstants::kRecaserOutput,
                 const std::string& lmkey = HifstConstants::kRecaserLmLoad,
                 const std::string& unimapkey = HifstConstants::kRecaserUnimapLoad,
                 bool forceloading = false ) :
    rg_ ( rg ),
    lmkey_ ( lmkey ),
    inputkey_ ( inputkey ),
    outputkey_ ( outputkey ),
    unimapkey_ ( unimapkey ),
    unimap_ ( NULL ) {
    using ucam::util::toNumber;
    // Prune or shortest path...
    std::vector<std::string> pstrat = rg_.getVectorString (
                                        HifstConstants::kRecaserPrune );
    USER_CHECK ( pstrat.size() == 2,
                 "prune parameter must be byshortestpath/byweight,number" );
    if ( pstrat[0] == "byshortestpath" ) {
      LINFO ( "Shortest Path n=" << pstrat[1] );
      shp_ = toNumber<unsigned> ( pstrat[1] );
      prune_ = std::numeric_limits<float>::max();
    } else if ( pstrat[0] == "byweight" ) {
      LINFO ( "Prune by weight b=" << pstrat[1] );
      shp_ = std::numeric_limits<unsigned>::max();
      prune_ = toNumber<float> ( pstrat[1] );
    } else {
      USER_CHECK ( false,
                   "prune parameter incorrectly set: first parameter is byshortestpath or byweight" );
    }
    if ( rg.get<std::string> ( lmkey_) == ""
         && rg.get<std::string> ( unimapkey_ ) == "" ) return;
    if ( ! USER_CHECK ( rg.get<std::string> ( lmkey_ ) != ""
                        && rg.get<std::string> ( unimapkey_ ) != "" ,
                        "recaser.lm and recaser.unimap must either be both defined or both left to empty string " ) )
      return;
  };

  ///Method inherited from TaskInterface, reads input lattice from data object, disambiguates and then stores lattice in data object.
  bool run ( Data& d ) {
    d_ = &d;
    USER_CHECK ( d.fsts.find ( inputkey_ ) != d.fsts.end(),
                 "No input fst to recase?" );
    fst::ShortestPath<Arc> ( * ( static_cast< fst::VectorFst<Arc> * >
                                 (d.fsts[inputkey_] ) ), &olattice_, 1 );
    fst::Map<Arc> ( &olattice_, fst::RmWeightMapper<Arc>() );
    run ( &olattice_ );
    LINFO ( "(Recased) lattice available at key=" << outputkey_ );
    d.fsts[outputkey_] = &olattice_;
    return false;
  };

  virtual ~DisambigTask() { }

 private:
  typedef fst::ApplyLanguageModelOnTheFlyInterface<Arc> ApplyLanguageModelOnTheFlyInterfaceType;
  typedef boost::shared_ptr<ApplyLanguageModelOnTheFlyInterfaceType> ApplyLanguageModelOnTheFlyInterfacePtrType;
  ApplyLanguageModelOnTheFlyInterfacePtrType almotf_;


  // Initializes appropriate templated kenlm handler for composition
  // TODO: this code can be merged with task.applylm and task.hifst
  void initializeLanguageModelHandler() {
    if (almotf_.get() )  return; // already initialized
    USER_CHECK ( d_->klm.find ( lmkey_ ) != d_->klm.end() 
		 && d_->klm[lmkey_].size() == 1
                 , "You need to load ONE recasing Language Model!" );
    fst::MakeWeight<Arc> mw;
    unordered_set<Label> epsilons;
    ///We want the language model to ignore these guys:
    epsilons.insert ( DR );
    epsilons.insert ( OOV );
    epsilons.insert ( EPSILON );
    epsilons.insert ( SEP );
    bool a = true;
    almotf_.reset(assignKenLmHandler<Arc>
		  ( rg_, lmkey_, epsilons, *(d_->klm[lmkey_][0]), mw, a));
  }

  /// Actual disambiguation done here: first apply unimap model, then apply language model.
  void run ( fst::VectorFst<Arc> *fst ) {
    if ( d_->fsts.find ( unimapkey_ ) == d_->fsts.end() ) {
      LINFO ( "No recasing step (key=" << unimapkey_ << " not found)" );
      return;
    } else if ( d_->fsts[unimapkey_] == NULL ) {
      LINFO ( "No recasing step (NULL) " );
      return;
    }
    initializeLanguageModelHandler();
    unimap_ = static_cast<fst::VectorFst<Arc> *> ( d_->fsts[unimapkey_] );
    LINFO ( "Apply Unigram Model to 1-best" );
    fst::VectorFst<Arc> mappedinput ( fst::RRhoCompose<Arc> ( *fst, *unimap_ ) );
    LINFO ( "Tag OOVs" );
    tagOOVs<Arc> ( &mappedinput, *d_->recasingvcblm );
    LDBG_EXECUTE ( mappedinput.Write ( "mappedinput.fst" ) );
    fst::VectorFst<Arc> *output = almotf_->run(mappedinput);
    LINFO ( "Recover OOVs" );
    recoverOOVs<Arc> ( output );
    if ( shp_ < std::numeric_limits<unsigned>::max() ) {
      fst::VectorFst<Arc> *aux = new fst::VectorFst<Arc>;
      LINFO ( "Shortest Path n=" << shp_ );
      fst::ShortestPath<Arc> ( *output, aux, shp_ );
      delete output; output = aux;
      fst::TopSort<Arc> ( output );
    } else if ( prune_ < std::numeric_limits<float>::max() ) {
      LINFO ( "Prune by weight=" << prune_ );
      fst::Prune<Arc> ( output, mw_ ( prune_ ) );
    } else {
      USER_CHECK ( false,
                   "prune parameter incorrectly set: first parameter is byshortestpath or byweight" );
      exit(EXIT_FAILURE);
    }
    *fst = *output; delete output;
    fst::Project ( fst, fst::PROJECT_OUTPUT ); //Recased symbols on output language.
  }

  ZDISALLOW_COPY_AND_ASSIGN ( DisambigTask );

};

}
} // end namespaces
#endif // TASK_DISAMBIG_HPP

