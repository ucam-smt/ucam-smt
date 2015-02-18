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

#ifndef RUNNER_ALILATS2SPARSELATS_HPP
#define RUNNER_ALILATS2SPARSELATS_HPP

/**
 * \file
 * \brief Implements single-threaded version of alilats2splats tool
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Sets scales using environment parameter (see sparse tuple weight semiring file), or
 *        grammar scales and language model scales. If these are active, the environment
 *        parameter will not be used.
 * \param offset           The number of language models will be stored here, as an offset for the grammar scales in the sparse tropical tuple weight semiring
 * \param lmscales         Key to access registry object with  commandline parameter for language model scales
 * \param grammarscales    Key to access registry object with  commandline parameter for grammar scales
 */

void setScales ( const ucam::util::RegistryPO& rg,
                 unsigned *offset,
                 const std::string& lmscales = HifstConstants::kLmFeatureweights,
                 const std::string& grammarscales =
                   HifstConstants::kRuleflowerlatticeFeatureweights,
                 const std::string& featureweights = HifstConstants::kFeatureweights,
                 const std::string& lmload = HifstConstants::kLmLoad
               ) {
  if (rg.get<std::string> (featureweights) != "" ) {
    // overrides separate grammar feature weights + lm feature weights
    fst::TropicalSparseTupleWeight<float>::Params()
      = ucam::util::ParseParamString<float> (rg.getString (featureweights) );
    *offset = rg.getVectorString (lmload).size();
    LWARN ( HifstConstants::kFeatureweights << " overrides program options " <<
            HifstConstants::kRuleflowerlatticeFeatureweights << " and " <<
            HifstConstants::kLmFeatureweights);
    return;
  }
  std::vector<float> fscales1, fscales2;
  if ( rg.exists ( grammarscales ) )
    if ( rg.get<std::string> ( grammarscales ) != "" )
      fscales2 = ucam::util::ParseParamString<float> ( rg.getString (
                   grammarscales ) );
  if ( rg.exists ( lmscales ) )
    if ( rg.get<std::string> ( lmscales ) != "" )
      fscales1 = ucam::util::ParseParamString<float> ( rg.getString ( lmscales ) );
  std::vector<float>& fscales = fst::TropicalSparseTupleWeight<float>::Params();
  *offset = fscales1.size();
  if ( fscales1.size() + fscales2.size() ) {
    LWARN ( "env parameter is overriden by " << lmscales << "," << grammarscales );
    fscales.clear();
    fscales = fscales1;
    copy ( fscales2.begin(), fscales2.end(), std::back_inserter ( fscales ) );
  }
  USER_CHECK ( fscales.size(),
               "Number of scaling factors  must be greater than 0" );
  ///Dump scales
  std::string x = ucam::util::toString<float> ( fscales[0] );
  for ( unsigned k = 1; k < fscales.size(); ++k )
    x += "," + ucam::util::toString<float> ( fscales[k] );
  LINFO ( "Number of language models =" << *offset << ". Scales=" << x );
};

/**
 * \brief Full single-threaded Alignment lattices to Sparse lattices
 */

template < template <class> class DataT
           , class ArcT = void
           >
class SingleThreadedAliLatsToSparseVecLatsTask: public
ucam::util::TaskInterface<DataT<TupleArc32 > > {
 private:
  typedef DataT<TupleArc32 > Data;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::WriteFstTask < Data , TupleArc32 > WriteFst;
  typedef ucam::fsttools::ReadFstTask < Data , fst::LexStdArc > ReadFst;
  typedef ucam::fsttools::LoadLanguageModelTask < Data >
  LoadLanguageModel;
  typedef LoadSparseWeightFlowerLatticeTask < Data >
  LoadSparseWeightFlowerLattice;
  typedef SparseWeightVectorLatticesTask <Data, fst::LexStdArc >
  SparseWeightVectorLattices;
  typedef ucam::fsttools::ApplyLanguageModelTask<Data, TupleArc32 >
  ApplyLanguageModel;
  typedef DumpNbestFeaturesTask < Data  > DumpNbestFeatures;

  const ucam::util::RegistryPO& rg_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to ucam::util::RegistryPO object with all parsed parameters.
   */
  SingleThreadedAliLatsToSparseVecLatsTask ( const ucam::util::RegistryPO& rg ) :
    rg_ ( rg ) {
  };

  /**
   * \brief Reads an input sentence, tokenizes and integer-maps.
   */
  bool run ( Data& d ) {
    using namespace HifstConstants;
    unsigned numlms;
    setScales ( rg_ , &numlms);
    boost::scoped_ptr < LoadSparseWeightFlowerLattice>
    mytask (new LoadSparseWeightFlowerLattice ( rg_ , numlms,
            kSparseweightvectorlatticeLoadalilats ) );
    mytask->appendTask
    ( WriteFst::init ( rg_, kRuleflowerlatticeStore ) )
    ( LoadWordMap::init ( rg_, kLmWordmap, true ) )
    ( new LoadLanguageModel ( rg_, kLmLoad, "" ) ) //Here, language model weights always to 1
    ( new ReadFst ( rg_, kSparseweightvectorlatticeLoadalilats ) )
    ( new SparseWeightVectorLattices ( rg_
                                       , kSparseweightvectorlatticeLoadalilats
                                       , kRuleflowerlatticeStore
                                       , kSparseweightvectorlatticeStorenolm ) )
    ( new ApplyLanguageModel ( rg_
                               , kLmLoad
                               , kSparseweightvectorlatticeStorenolm
                               , kSparseweightvectorlatticeStore ) )
    ( WriteFst::init ( rg_, kSparseweightvectorlatticeStore ) )
    ( LoadWordMap::init ( rg_, kSparseweightvectorlatticeWordmap ) )
    ( DumpNbestFeatures::init ( rg_
                                , numlms
                                , kSparseweightvectorlatticeStore ) )
    ;
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                      kRangeOne ) );
          !ir->done ();
          ir->next () ) {
      d.sidx = ir->get ();
      mytask->chainrun ( d ); // Run!
    }
    return false;
  };

  inline bool operator() () {
    Data d;
    return run ( d );
  };

 private:

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadedAliLatsToSparseVecLatsTask );

};

/**
 * \brief Multithreaded implementation of alilats2splats pipeline
 */
template < template <class> class DataT
           , class ArcT = void
           >
class MultiThreadedAliLatsToSparseVecLatsTask: public
ucam::util::TaskInterface<DataT<TupleArc32 > > {
 private:
  typedef DataT<TupleArc32 > Data;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::WriteFstTask < Data , TupleArc32 > WriteFst;
  typedef ucam::fsttools::ReadFstTask < Data , fst::LexStdArc > ReadFst;
  typedef ucam::fsttools::LoadLanguageModelTask < Data >
  LoadLanguageModel;
  typedef LoadSparseWeightFlowerLatticeTask < Data >
  LoadSparseWeightFlowerLattice;
  typedef SparseWeightVectorLatticesTask <Data, fst::LexStdArc >
  SparseWeightVectorLattices;
  typedef ucam::fsttools::ApplyLanguageModelTask<Data, TupleArc32 >
  ApplyLanguageModel;
  typedef DumpNbestFeaturesTask < Data  > DumpNbestFeatures;

  ///Registry object
  const ucam::util::RegistryPO& rg_;
  ///Number of threads requested by user
  unsigned threadcount_;
 public:
  MultiThreadedAliLatsToSparseVecLatsTask ( const ucam::util::RegistryPO& rg ) :
    threadcount_ ( rg.get<unsigned> ( HifstConstants::kNThreads ) ),
    rg_ (rg) {
  }

  bool run ( Data& original_data ) {
    using namespace HifstConstants;
    unsigned numlms;
    setScales ( rg_ , &numlms);
    boost::scoped_ptr < LoadSparseWeightFlowerLattice>
    loadtask ( new LoadSparseWeightFlowerLattice ( rg_
               , numlms
               , kSparseweightvectorlatticeLoadalilats ) );
    loadtask->appendTask
    ( WriteFst::init ( rg_  , kRuleflowerlatticeStore ) )
    ( new LoadLanguageModel ( rg_  , kLmLoad,
                              "" ) ) //Forcing language model scales always to 1
    ( LoadWordMap::init ( rg_  , kLmWordmap , true ) )
    ( LoadWordMap::init ( rg_  ,
                          kSparseweightvectorlatticeWordmap ) )
    ;
    loadtask->chainrun ( original_data ); // Load grammar and language model;
    {
      ucam::util::TrivialThreadPool tp ( threadcount_ );
      bool finished = false;
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                       kRangeOne ) );
            !ir->done ();
            ir->next () ) {
        Data *d = new Data; //( original_data ); // reset.
        d->sidx = ir->get();
        d->klm = original_data.klm;
        d->fsts = original_data.fsts;
        d->wm = original_data.wm;
        FORCELINFO ( "=====Extract features for sentence " << d->sidx << ":" );
        ReadFst *runtask = new ReadFst ( rg_  ,
                                         kSparseweightvectorlatticeLoadalilats );
        runtask->appendTask
        ( new SparseWeightVectorLattices ( rg_
                                           , kSparseweightvectorlatticeLoadalilats
                                           , kRuleflowerlatticeStore
                                           , kSparseweightvectorlatticeStorenolm ) )
        ( new ApplyLanguageModel ( rg_
                                   , kLmLoad
                                   , kSparseweightvectorlatticeStorenolm
                                   , kSparseweightvectorlatticeStore ) )
        ( WriteFst::init ( rg_  , kSparseweightvectorlatticeStore ) )
        ( DumpNbestFeatures::init ( rg_  , numlms,
                                    kSparseweightvectorlatticeStore ) )
        ;
        tp ( ucam::util::TaskFunctor<Data> ( runtask,
                                             d ) ); //tp takes ownership of runtask and d
      }
    }
    return false;
  };

  inline bool operator() () {
    Data d;
    return run ( d );
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( MultiThreadedAliLatsToSparseVecLatsTask );
};

}
} // end namespaces

#endif
