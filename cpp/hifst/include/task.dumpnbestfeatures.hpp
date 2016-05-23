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

#ifndef TASK_DUMPNBESTFEATURES_HPP
#define TASK_DUMPNBESTFEATURES_HPP

/**
 * \file
 * \brief Contains task that dumps nbest and feature file
 * \date 22-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {
/**
 * \brief Task that dumps nbest and feature file. Templated on specific
 * Data object and Fst Arc
 */
template<class Data, class Arc = fst::StdArc>
class DumpNbestFeaturesTask: public ucam::util::TaskInterface<Data> {
 private:

  ///Feature file name
  ucam::util::IntegerPatternAddress fdir_;
  ///Nbest file name
  ucam::util::IntegerPatternAddress nbdir_;

  ///Wordmap for target words
  ucam::util::WordMapper *wm_;

  ///Scales used with the sparse tuple weight
  std::vector<float>& scales_;

  ///Key to access in registry object the
  const std::string sparseweightlatticekey_;

  ///Feature indices higher than "trigger_" will be printed using
  /// sparse representation (i.e. printing feature  as weight@position)
  const unsigned trigger_;

  /// How many context parameters (i.e. language models)
  /// are there before the grammar scales themselves
  /// Language model scores contribute in first offset_ lower indices (starting at 1)
  const unsigned offset_;

  const std::string wordmapkey_;

 public:

  /**
   * \brief Constructor
   * \param  rg                             RegistryPO object containing program options
   * \param offset                          Number of language models
   * \param sparseweightlatticekey          Key to access parseweightlattice in the data object
   */

  DumpNbestFeaturesTask ( const ucam::util::RegistryPO& rg ,
                          const unsigned offset = 1, //Minimum for 1 language model
                          const std::string& sparseweightlatticekey =
                            HifstConstants::kSparseweightvectorlatticeStore ,
                          const std::string& wordmapkey =
                            HifstConstants::kSparseweightvectorlatticeWordmap
                        ) :
    offset_ ( offset ),
    sparseweightlatticekey_ ( sparseweightlatticekey ),
    fdir_ ( rg.get<std::string>
            ( HifstConstants::kSparseweightvectorlatticeStorefeaturefile ) ),
    nbdir_ ( rg.get<std::string>
             ( HifstConstants::kSparseweightvectorlatticeStorenbestfile ) ),
    wm_ (NULL),
    scales_ ( fst::TropicalSparseTupleWeight<float>::Params() ),
    trigger_ ( rg.get<unsigned>
               ( HifstConstants::kSparseweightvectorlatticeFirstsparsefeatureatindex ) ),
    wordmapkey_ ( wordmapkey ) {
  };

  ///Static method to create and object of this task class. Parameters identical to constructor.
  ///It will return NULL no program option using this task has been defined by the program user.
  static DumpNbestFeaturesTask* init ( const ucam::util::RegistryPO& rg,
                                       const unsigned offset = 1 ,
                                       const std::string& sparseweightlatticekey =
                                         HifstConstants::kSparseweightvectorlatticeStore
                                     ) {
    if ( rg.exists ( HifstConstants::kSparseweightvectorlatticeStorefeaturefile )
         || rg.exists ( HifstConstants::kSparseweightvectorlatticeStorenbestfile ) )
      return new DumpNbestFeaturesTask ( rg, offset, sparseweightlatticekey );
    return NULL;
  }

  /// Dumps nbest or features from sparse tuple weight lattice in the data object d, if it exists
  bool run ( Data& d ) {
    //Search for tuplearc32 lattice
    if ( d.fsts.find ( sparseweightlatticekey_ ) == d.fsts.end() ) {
      LWARN ( "No sparse tuple lattice to dump!" );
      return true;
    }
    fst::VectorFst<TupleArc32> *vectorlattice =
      static_cast<fst::VectorFst<TupleArc32> *> ( d.fsts[sparseweightlatticekey_] );
    if ( nbdir_() != "" ) {
      if ( d.wm.find ( wordmapkey_ ) != d.wm.end() )
        wm_ = d.wm[wordmapkey_];
      else wm_ = NULL;
      writeNbestFile ( *vectorlattice, nbdir_ ( d.sidx ) );
    }
    if ( fdir_() != "" )
      writeFeatureFile ( *vectorlattice, fdir_ ( d.sidx ) );
    return false;
  }

  ///Specific method to write nbest list from the sparse vector weight lattice
  void writeNbestFile ( const fst::VectorFst<TupleArc32>& vectorlattice,
                        const std::string& filename ) {
    USER_CHECK ( vectorlattice.NumStates()
                 , "Attempting to write an nbest file from an empty lattice!" );
    unordered_map<std::string, float> hyps;
    unordered_map<std::string, std::string> fhyps;
    fst::VectorFst<Arc> lattice;
    fst::DotProductMap<float> m ( scales_ );
    fst::Map ( vectorlattice
               , &lattice
               , fst::GenericWeightMapper<TupleArc32, Arc, fst::DotProductMap<float>  >
               ( m ) );
    fst::printstrings<Arc> ( lattice, hyps );
    std::priority_queue<struct fst::hypcost, std::vector< struct fst::hypcost >, fst::CompareHyp>
          pq;
    for ( unordered_map<std::string, float>::iterator itx = hyps.begin()
          ; itx != hyps.end()
          ; ++itx ) {
      struct fst::hypcost hc;
      if (wm_ != NULL) (*wm_) (itx->first, &hc.hyp, false);
      else hc.hyp = itx->first;
      boost::algorithm::trim ( hc.hyp );
      hc.cost = itx->second;
      pq.push ( hc );
    }
    FORCELINFO ( "Write Nbest File to " << filename );
    ucam::util::oszfstream file ( filename );
    while ( !pq.empty() ) {
      struct fst::hypcost hc = pq.top();
      file << hc.hyp << "\t" << hc.cost << std::endl;
      pq.pop();
    }
    file.close();
  }

  ///Specific method to dump feature file from sparse vector weight lattice.
  void writeFeatureFile ( const fst::VectorFst<TupleArc32>& vectorlattice ,
                          const std::string& filename ) {
    USER_CHECK ( vectorlattice.NumStates(),
                 "Attempting to write feature file for an empty lattice!!!!" );
    unordered_map<std::string, float> hyps;
    unordered_map<std::string, std::string> fhyps;
    fst::VectorFst<Arc> lattice;
    fst::DotProductMap<float> m ( scales_ );
    fst::Map ( vectorlattice, &lattice,
               fst::GenericWeightMapper<TupleArc32, Arc, fst::DotProductMap<float>  > ( m ) );
    fst::printstrings<Arc> ( lattice, hyps );
    fst::VectorFst<TupleArc32> vl ( vectorlattice );
    std::unordered_set<unsigned> latticefeatures;
    listSparseFeatureIndices ( vl, latticefeatures );
    LINFO ( "Number of Active features:" << latticefeatures.size() );
    for ( unsigned k = 0; k < scales_.size(); ++k ) {
      if ( k + 1 > trigger_
           && latticefeatures.find ( k + 1 ) == latticefeatures.end() ) continue;
      fst::VectorFst<Arc> dimfst;
      {
        fst::VectorToStd<float> m ( k );
        fst::Map ( vl, &dimfst,
                   fst::GenericWeightMapper<TupleArc32, Arc, fst::VectorToStd<float> > ( m ) );
      }
      unordered_map<std::string, float> dhyps;
      fst::printstrings<Arc> ( dimfst, dhyps );
      using ucam::util::toString;
      for ( unordered_map<std::string, float>::iterator itx = dhyps.begin();
            itx != dhyps.end(); ++itx ) {
        if ( k + 1 > trigger_
             && itx->second == 0.0000f ) continue; //skip features with path cost 0
        if ( fhyps.find ( itx->first ) != fhyps.end() ) fhyps[itx->first] += "\t" +
              toString<float> ( itx->second, 4 );
        else fhyps[itx->first] = toString<float> ( itx->second, 4 );
        if ( k + 1 > trigger_ ) fhyps[itx->first] += "@" + toString<unsigned>
              ( k ); //Add @position.
      }
    }
    std::priority_queue<struct fst::hypcost, std::vector< struct fst::hypcost >, fst::CompareHyp>
          pq;
    for ( unordered_map<std::string, float>::iterator itx = hyps.begin()
          ; itx != hyps.end()
          ; ++itx ) {
      struct fst::hypcost hc;
      hc.hyp = fhyps[itx->first];
      hc.cost = itx->second;
      pq.push ( hc );
    }
    FORCELINFO ( "Write Feature File to " << filename );
    ucam::util::oszfstream file ( filename );
    while ( !pq.empty() ) {
      struct fst::hypcost hc = pq.top();
      file << hc.hyp << std::endl;
      pq.pop();
    }
    file.close();
  }
};

}
} // end namespaces

#endif // TASK_DUMPNBESTFEATURES_HPP
