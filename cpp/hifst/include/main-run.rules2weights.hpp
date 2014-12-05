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

#pragma once

/**
 * \file
 * \brief Implements lats2splats tool
 * \date 2-12-2014
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Full single-threaded Alignment lattices to Sparse lattices
 */

template <class Data = RuleIdsToWeightsData<lm::ngram::Model>
          , class KenLMModelT = lm::ngram::Model >
class SingleThreadedRulesToWeightsSparseLatsTask: public
  ucam::util::TaskInterface<Data> {
 private:
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::WriteFstTask < Data , TupleArc32 > WriteFst;
  typedef ucam::fsttools::ReadFstTask < Data , fst::LexStdArc > ReadFst;
  typedef ucam::fsttools::LoadLanguageModelTask < Data, KenLMModelT >
  LoadLanguageModel;
  typedef LoadSparseWeightFlowerLatticeTask < Data >
  LoadSparseWeightFlowerLattice;
  typedef SparseWeightVectorLatticesTask <Data, fst::LexStdArc >
  SparseWeightVectorLattices;
  typedef ucam::fsttools::ApplyLanguageModelTask<Data, KenLMModelT, TupleArc32 >
  ApplyLanguageModel;
  typedef DumpNbestFeaturesTask < Data  > DumpNbestFeatures;

  const ucam::util::RegistryPO& rg_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to ucam::util::RegistryPO object with all parsed parameters.
   */
  SingleThreadedRulesToWeightsSparseLatsTask ( const ucam::util::RegistryPO& rg ) :
    rg_ ( rg ) {
  };

  /**
   * \brief Reads an input sentence, tokenizes and integer-maps.
   */
  bool run ( Data& d ) {
    unsigned numlms;
    setScales ( rg_ , &numlms);
    boost::scoped_ptr < LoadSparseWeightFlowerLattice>
    mytask (new LoadSparseWeightFlowerLattice ( rg_ , numlms,
            HifstConstants::kSparseweightvectorlatticeLoadalilats ) );
    mytask->appendTask
    ( WriteFst::init ( rg_, HifstConstants::kRuleflowerlatticeStore ) )
    ( LoadWordMap::init ( rg_, HifstConstants::kLmWordmap, true ) )
    ( new LoadLanguageModel ( rg_, HifstConstants::kLmLoad,
                              "" ) ) //Here, language model weights always to 1
    ( new ReadFst ( rg_
                    , HifstConstants::kSparseweightvectorlatticeLoadalilats ) )
    ( new SparseWeightVectorLattices ( rg_
                                       , HifstConstants::kSparseweightvectorlatticeLoadalilats
                                       , HifstConstants::kRuleflowerlatticeStore
                                       , HifstConstants::kSparseweightvectorlatticeStorenolm ) )
    ( new ApplyLanguageModel ( rg_
                               , HifstConstants::kLmLoad
                               , HifstConstants::kSparseweightvectorlatticeStorenolm
                               , HifstConstants::kSparseweightvectorlatticeStore ) )
    ( WriteFst::init ( rg_
                       , HifstConstants::kSparseweightvectorlatticeStore ) )
    ( LoadWordMap::init ( rg_
                          , HifstConstants::kSparseweightvectorlatticeWordmap ) )
    ( DumpNbestFeatures::init ( rg_
                                , numlms
                                , HifstConstants::kSparseweightvectorlatticeStore ) )
    ;
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                      HifstConstants::kRangeOne ) );
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

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadedRulesToWeightsSparseLatsTask );

};

/**
 * \brief Multithreaded implementation of alilats2splats pipeline
 */
template <class Data = RuleIdsToWeightsData<lm::ngram::Model> , class KenLMModelT = lm::ngram::Model  >
class MultiThreadedRulesToWeightsSparseLatsTask: public
  ucam::util::TaskInterface<Data> {

 private:
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::WriteFstTask < Data , TupleArc32 > WriteFst;
  typedef ucam::fsttools::ReadFstTask < Data , fst::LexStdArc > ReadFst;
  typedef ucam::fsttools::LoadLanguageModelTask < Data, KenLMModelT >
  LoadLanguageModel;
  typedef LoadSparseWeightFlowerLatticeTask < Data >
  LoadSparseWeightFlowerLattice;
  typedef SparseWeightVectorLatticesTask <Data, fst::LexStdArc >
  SparseWeightVectorLattices;
  typedef ucam::fsttools::ApplyLanguageModelTask<Data, KenLMModelT, TupleArc32 >
  ApplyLanguageModel;
  typedef DumpNbestFeaturesTask < Data  > DumpNbestFeatures;

  ///Registry object
  const ucam::util::RegistryPO& rg_;
  ///Number of threads requested by user
  unsigned threadcount_;
 public:
  MultiThreadedRulesToWeightsSparseLatsTask ( const ucam::util::RegistryPO& rg ) :
    threadcount_ ( rg.get<unsigned> ( HifstConstants::kNThreads ) ),
    rg_ (rg) {
  }

  bool run ( Data& original_data ) {
    unsigned numlms;
    setScales ( rg_ , &numlms);
    boost::scoped_ptr < LoadSparseWeightFlowerLattice>
    loadtask ( new LoadSparseWeightFlowerLattice ( rg_
               , numlms
               , HifstConstants::kSparseweightvectorlatticeLoadalilats ) );
    loadtask->appendTask
    ( WriteFst::init ( rg_  , HifstConstants::kRuleflowerlatticeStore ) )
    ( new LoadLanguageModel ( rg_  , HifstConstants::kLmLoad,
                              "" ) ) //Forcing language model scales always to 1
    ( LoadWordMap::init ( rg_  , HifstConstants::kLmWordmap , true ) )
    ( LoadWordMap::init ( rg_  ,
                          HifstConstants::kSparseweightvectorlatticeWordmap ) )
    ;
    loadtask->chainrun ( original_data ); // Load grammar and language model;
    {
      ucam::util::TrivialThreadPool tp ( threadcount_ );
      bool finished = false;
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                       HifstConstants::kRangeOne ) );
            !ir->done ();
            ir->next () ) {
        Data *d = new Data; //( original_data ); // reset.
        d->sidx = ir->get();
        d->klm = original_data.klm;
        d->fsts = original_data.fsts;
        d->wm = original_data.wm;
        FORCELINFO ( "=====Extract features for sentence " << d->sidx << ":" );
        ReadFst *runtask = new ReadFst ( rg_  ,
                                         HifstConstants::kSparseweightvectorlatticeLoadalilats );
        runtask->appendTask
        ( new SparseWeightVectorLattices ( rg_
                                           , HifstConstants::kSparseweightvectorlatticeLoadalilats
                                           , HifstConstants::kRuleflowerlatticeStore
                                           , HifstConstants::kSparseweightvectorlatticeStorenolm ) )
        ( new ApplyLanguageModel ( rg_
                                   , HifstConstants::kLmLoad
                                   , HifstConstants::kSparseweightvectorlatticeStorenolm
                                   , HifstConstants::kSparseweightvectorlatticeStore ) )
        ( WriteFst::init ( rg_  , HifstConstants::kSparseweightvectorlatticeStore ) )
        ( DumpNbestFeatures::init ( rg_  , numlms,
                                    HifstConstants::kSparseweightvectorlatticeStore ) )
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
  DISALLOW_COPY_AND_ASSIGN ( MultiThreadedRulesToWeightsSparseLatsTask );
};
}} // end namespaces

