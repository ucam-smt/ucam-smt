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

#include <szfstream.hpp>

/**
 * \file
 * \brief Core implementation of applylm binary.
 * Kicks off either singlethreaded or multithreaded language model application
 * \date September 2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

namespace HC = HifstConstants;

// Create a vector of source windows given an input sentence.
// Assume odd sizes (1,3,...)
inline void createSourceWindows(std::string const &integerMappedSentence
                                , unsigned srcSize
                                , std::vector< std::vector<unsigned> > &srcw) {

  LINFO("Creating source window...");
  if (srcSize && !(srcSize % 2 )) {
    LERROR("Only 0 or odd source sizes (1,3,...)");
    exit(EXIT_FAILURE);
  }
  if (!srcSize) return;
  using namespace boost::algorithm;
  std::deque<std::string> words;
  split(words, integerMappedSentence,is_any_of(" "));
  srcw.clear();
  srcw.resize(words.size());
  // assume that we already have <s> and </s> (i.e. 1,2).
  for (unsigned k = 0; k < srcSize/2; ++k) {
    words.push_front("1");
    words.push_back("2");
  }

  using ucam::util::toNumber;
  for (unsigned k = 0; k < words.size() - srcSize + 1; ++k) {
    srcw[k].push_back(toNumber<unsigned>(words[k]));
    std::vector<unsigned> &aux = srcw[k];
    for (unsigned j = 1; j < srcSize; ++j) {
      aux.push_back(toNumber<unsigned>(words[ k + j ]));
    }
  }
#ifdef PRINTDEBUG1
  // Print original src words here:
  for (unsigned k = 0; k < srcw.size(); ++k) {
    std::stringstream ss; ss << srcw[k][0];
    for (unsigned j = 1; j < srcw[k].size(); ++j ){
      ss << " " << srcw[k][j];
    }
    LDEBUG("*** ORIGINAL src words=" << ss.str());
  }
#endif
};

template< class ArcT, template<class> class DataT >
ucam::util::TaskInterface< DataT<ArcT>  > *addApplyLM
(bool bilm, ucam::util::RegistryPO const& rg) {

  typedef DataT<ArcT> Data;
  typedef ApplyLanguageModelTask< Data, ArcT > ApplyLanguageModel;
  typedef ApplyBiLMTask< Data, ArcT > ApplyBiLM;
  using namespace HifstConstants;

  if (!bilm)
    return new ApplyLanguageModel
        ( rg
          , kLmLoad
          , kLatticeLoad
          , kLatticeStore
          , rg.exists (kLatticeLoadDeleteLmCost) )
        ;

  return new ApplyBiLM
      ( rg
        , kLmLoad
        , kLatticeLoad
        , kLatticeStore
        , rg.exists (kLatticeLoadDeleteLmCost) )
      ;
};

/**
 * \brief Class for single threaded application of language model.
 * It inherits taskinterface behaviour and also provides
 * standalone function object behaviour.
 */
template < template <class> class DataT
           , class ArcT
           >
class SingleThreadedApplyLanguageModelTask
    : public ucam::util::TaskInterface<DataT<ArcT>  > {
 private:
  typedef DataT<ArcT> Data;
  typedef LoadWordMapTask< Data > LoadWordMap;
  typedef LoadLanguageModelTask < Data > LoadLanguageModel;
  typedef ReadFstTask<Data, ArcT> ReadFst;
  typedef WriteFstTask< Data , ArcT> WriteFst;
  typedef TuneWpWriteFstTask< Data , ArcT> TuneWpWriteFst;
  typedef SpeedStatsTask<Data> SpeedStats;
  typedef ucam::util::iszfstream iszfstream;
  typedef ucam::util::FastForwardRead<iszfstream> FastForwardRead;
  typedef boost::scoped_ptr<FastForwardRead> FastForwardReadPtr;

  // Use bilingual model or not
  bool bilm_;
  // Source window size for bilingual models:
  unsigned srcSize_;
  // source file
  std::string srcFile_;
  // Convenience file reading with range parameter:
  FastForwardReadPtr fastForwardRead_;
  //Command-line/config file options
  ucam::util::RegistryPO const& rg_;

 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  SingleThreadedApplyLanguageModelTask ( ucam::util::RegistryPO const& rg )
      : bilm_(rg.getBool(HC::kUseBilingualModel))
      , srcSize_(rg.get<unsigned>(HC::kUseBilingualModelSourceSize))
      , srcFile_(rg.get<string>(HC::kUseBilingualModelSourceSentenceFile))
      , rg_ ( rg )
  {
    if (bilm_)
      if (srcFile_ != "")
        fastForwardRead_.reset(new FastForwardRead
                               ( new iszfstream ( srcFile_) ));
      else {
        LERROR("Parameter " << HC::kUseBilingualModelSourceSentenceFile
               << " needs to be defined");
        exit(EXIT_FAILURE);
      }
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
    using namespace HifstConstants;
    boost::scoped_ptr< LoadWordMap > tasks
        (new LoadWordMap (rg_, kLmWordmap, true) );
    tasks->appendTask
        ( new LoadLanguageModel ( rg_ ) )
        ( new ReadFst ( rg_ , kLatticeLoad ) )
        ( addApplyLM<ArcT,DataT>(bilm_, rg_ ) )
        ( WriteFst::init ( rg_ , kLatticeStore ) )
        ( TuneWpWriteFst::init( rg_, kTuneWrite, kLatticeStore ) )
        ( new SpeedStats ( rg_ ) )
        ;
    using namespace ucam::util;

    bool finished = false;
    Data d;
    for ( IntRangePtr ir
              (IntRangeFactory ( rg_, kRangeOne ) )
              ; !ir->done()
              ; ir->next() ) {
      d.sidx = ir->get();
      if (bilm_) {
        finished = (*fastForwardRead_) ( d.sidx , &d.integerMappedSentence);
        // could go to task.prepro, or even better, make it a task of its own,
        // so it's shareable.
        createSourceWindows(d.integerMappedSentence, srcSize_, d.sourceWindows);
      }
      LINFO ( "Running sentence " << d.sidx );
      tasks->chainrun ( d ); //Run!
      if (bilm_ && finished) break;
    }
    // let the next task run.
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
class MultiThreadedApplyLanguageModelTask
    : public ucam::util::TaskInterface<DataT<ArcT>  > {
 private:
  typedef DataT<ArcT> Data;
  typedef LoadWordMapTask< Data > LoadWordMap;
  typedef LoadLanguageModelTask < Data > LoadLanguageModel;
  typedef ApplyLanguageModelTask<Data,  ArcT > ApplyLanguageModel;
  typedef ReadFstTask<Data, ArcT> ReadFst;
  typedef WriteFstTask< Data , ArcT> WriteFst;
  typedef TuneWpWriteFstTask< Data , ArcT> TuneWpWriteFst;
  typedef SpeedStatsTask<Data> SpeedStats;

  typedef ucam::util::iszfstream iszfstream;
  typedef ucam::util::FastForwardRead<iszfstream> FastForwardRead;
  typedef boost::scoped_ptr<FastForwardRead> FastForwardReadPtr;

  // Use bilingual model or not
  bool bilm_;
  // Source window size for bilingual models:
  unsigned srcSize_;
  // source file
  std::string srcFile_;
  // Convenience file reading with range parameter:
  FastForwardReadPtr fastForwardRead_;
  ///Command-line/config file options
  ucam::util::RegistryPO const& rg_;
  ///Number of threads
  unsigned threadcount_;
 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  MultiThreadedApplyLanguageModelTask ( ucam::util::RegistryPO const& rg )
      : threadcount_ ( rg.get<unsigned> ( HC::kNThreads ) )
      , bilm_(rg.getBool(HC::kUseBilingualModel))
      , srcSize_(rg.get<unsigned>(HC::kUseBilingualModelSourceSize))
      , srcFile_(rg.get<string>(HC::kUseBilingualModelSourceSentenceFile))
      , rg_ ( rg )
  {
    if (bilm_)
      if (srcFile_ != "")
        fastForwardRead_.reset(new FastForwardRead
                               ( new iszfstream ( srcFile_) ));
      else {
        LERROR("Parameter " << HC::kUseBilingualModelSourceSentenceFile
               << " needs to be defined");
        exit(EXIT_FAILURE);
      }
  };

  ///Standalone behaviour, just call the functor and it will do the job.
  inline bool operator() () {
    Data d;
    return run ( d );
  }

  /**
   * \brief Multithread lm application.
   * Runs only if option --nthreads defined >0
   * \remarks Now a threadpool is defined, each
   * language model application is submitted as a thread.
   * Important note: in contrast to single-threading,
   * sentence specific language model loading
   * does not work with multithreading.
   * You have to load once your full language model.
   */
  bool run ( Data& original_data ) {
    using namespace HifstConstants;
    boost::scoped_ptr< LoadWordMap > mylm
        (new LoadWordMap (rg_, kLmWordmap, true) );
    mylm->appendTask ( new LoadLanguageModel ( rg_ ) );
    mylm->chainrun ( original_data ); //Loading language model only once
    {
      using namespace ucam::util;
      TrivialThreadPool tp ( threadcount_ );
      bool finished = false;
      for ( IntRangePtr ir(IntRangeFactory ( rg_, kRangeOne ) )
                ; !ir->done(); ir->next() ) {
        ReadFst *applylm =  new ReadFst ( rg_ , kLatticeLoad ) ;
        applylm->appendTask
            ( addApplyLM<ArcT,DataT>(bilm_, rg_ ) )
            ( WriteFst::init( rg_ , kLatticeStore ) )
            ( TuneWpWriteFst::init( rg_, kTuneWrite, kLatticeStore) )
            ( new SpeedStats ( rg_ ) )
            ;
        Data *d = new Data; //( original_data );
        d->klm = original_data.klm;
        d->sidx = ir->get();
        if (bilm_) {
          finished = (*fastForwardRead_) ( d->sidx , &d->integerMappedSentence);
          // could go to task.prepro, or even better, make it a task of its own,
          // so it's shareable.
          createSourceWindows(d->integerMappedSentence, srcSize_, d->sourceWindows);
        }
        LINFO ( "Running sentence " << d->sidx );
        tp ( TaskFunctor<Data> ( applylm, d ) );
        if (bilm_ && finished) break;
      }
    }
    return false;
  };
};

}} // end namespaces

#endif //MAIN_RUN_APPLYLM_HPP
