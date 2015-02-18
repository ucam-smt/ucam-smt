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

#ifndef MAIN_RUN_APPLYLM_HPP
#define MAIN_RUN_APPLYLM_HPP

/**
 * \file
 * \brief Core implementation of applylm binary. Kicks off either singlethreaded or multithreaded language model application
 * \date September 2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {
/**
 * \brief Class for single threaded application of language model.
 * It inherits taskinterface behaviour and also provides standalone function object behaviour.
 */
template < template <class> class DataT
           , class ArcT
           >
class SingleThreadedApplyLanguageModelTask: public ucam::util::TaskInterface<DataT<ArcT>  > {
 private:

  typedef DataT<ArcT> Data;
  typedef LoadWordMapTask< Data > LoadWordMap;
  typedef LoadLanguageModelTask < Data > LoadLanguageModel;
  typedef ApplyLanguageModelTask< Data, ArcT > ApplyLanguageModel;
  typedef ReadFstTask<Data, ArcT> ReadFst;
  typedef WriteFstTask< Data , ArcT> WriteFst;
  typedef SpeedStatsTask<Data> SpeedStats;

  //Command-line/config file options
  const ucam::util::RegistryPO& rg_;

 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  SingleThreadedApplyLanguageModelTask ( const ucam::util::RegistryPO& rg ) :
    rg_ ( rg ) {
  };

  /// Provides standalone behaviour (no external data object),
  /// just call the functor and it will do the job.
  void operator() () {
    Data d;
    run ( d );
  }

  /**
   * \brief Core function running language model application.
   * Creates list of tasks (load lm, apply lm) and executes.
   * \param original_data                 Data object
   */
  bool run ( Data& original_data ) {
    boost::scoped_ptr< LoadWordMap > tasks (new LoadWordMap (rg_,
                                            HifstConstants::kLmWordmap, true) );
    tasks->appendTask
    ( new LoadLanguageModel ( rg_ ) )
    ( new ReadFst ( rg_ , HifstConstants::kLatticeLoad ) )
    ( new ApplyLanguageModel ( rg_
                               , HifstConstants::kLmLoad
                               , HifstConstants::kLatticeLoad
                               , HifstConstants::kLatticeStore
                               , rg_.exists (HifstConstants::kLatticeLoadDeleteLmCost) ) )
    ( new WriteFst ( rg_ , HifstConstants::kLatticeStore ) )
    ( new SpeedStats ( rg_ ) )
    ;
    Data d;
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_,
                                      HifstConstants::kRangeOne ) )
          ; !ir->done()
          ; ir->next() ) {
      d.sidx = ir->get();
      LINFO ( "Running sentence " << d.sidx );
      tasks->chainrun ( d ); //Run!
    }
    return false;
  }
};

/**
 * \brief Class for multithreaded application of language model.
 * Inherits taskinterface and provides standalone function object behaviour.
 */
template < template <class> class DataT
           , class ArcT
           >
class MultiThreadedApplyLanguageModelTask: public ucam::util::TaskInterface<DataT<ArcT>  > {
 private:
  typedef DataT<ArcT> Data;
  typedef LoadWordMapTask< Data > LoadWordMap;
  typedef LoadLanguageModelTask < Data > LoadLanguageModel;
  typedef ApplyLanguageModelTask<Data,  ArcT > ApplyLanguageModel;
  typedef ReadFstTask<Data, ArcT> ReadFst;
  typedef WriteFstTask< Data , ArcT> WriteFst;
  typedef SpeedStatsTask<Data> SpeedStats;

  ///Command-line/config file options
  const ucam::util::RegistryPO& rg_;
  ///Number of threads
  uint threadcount_;
 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  MultiThreadedApplyLanguageModelTask ( const ucam::util::RegistryPO& rg ) :
    threadcount_ ( rg.get<uint> ( HifstConstants::kNThreads ) ),
    rg_ ( rg ) {
  };

  ///Standalone behaviour, just call the functor and it will do the job.
  inline bool operator() () {
    Data d;
    return run ( d );
  }

  /**
   * \brief Multithread lm application . Only if option --nthreads defined >0
   * \remarks Now a threadpool is defined, each language model application is submitted as a thread.
   * Important note: in contrast to single-threading, sentence specific language model loading
   * does not work with multithreading.  You have to load once your full language model.
   */
  bool run ( Data& original_data ) {
    boost::scoped_ptr< LoadWordMap > mylm (new LoadWordMap (rg_,
                                           HifstConstants::kLmWordmap, true) );
    mylm->appendTask ( new LoadLanguageModel ( rg_ ) );
    mylm->run ( original_data ); //Loading language model only once
    {
      ucam::util::TrivialThreadPool tp ( threadcount_ );
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_,
                                        HifstConstants::kRangeOne ) )
            ; !ir->done(); ir->next() ) {
        ReadFst *applylm =  new ReadFst ( rg_ , HifstConstants::kLatticeLoad ) ;
        applylm->appendTask
        ( new ApplyLanguageModel ( rg_
                                   , HifstConstants::kLmLoad
                                   , HifstConstants::kLatticeLoad
                                   , HifstConstants::kLatticeStore
                                   , rg_.exists (HifstConstants::kLatticeLoadDeleteLmCost) ) )
        ( new WriteFst ( rg_ , HifstConstants::kLatticeStore ) )
        ( new SpeedStats ( rg_ ) )
        ;
        Data *d = new Data; //( original_data );
        d->klm = original_data.klm;
        d->sidx = ir->get();
        LINFO ( "Running sentence " << d->sidx );
        tp ( ucam::util::TaskFunctor<Data> ( applylm, d ) );
      }
    }
    return false;
  };

};

}
} // end namespaces

#endif //MAIN_RUN_APPLYLM_HPP
