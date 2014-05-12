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

#ifndef MAIN_RUN_CREATESSGRAMMAR_HPP
#define MAIN_RUN_CREATESSGRAMMAR_HPP

/**
 * \file
 * \brief Contains createssgrammar core implementation, single-threaded or multithreaded
 * \date October 2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

using boost::asio::ip::tcp;
const int max_length = 1024;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

/**
 * \brief Full single-threaded Translation system
 */

template <class Data = HifstTaskData>
class SingleThreadedCreateSentenceSpecificGrammarTask: public
  ucam::util::TaskInterface<Data> {

  //Private variables are shown here. Private methods go after public methods
 private:
  typedef ucam::hifst::GrammarTask < Data > LoadGrammar;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::hifst::PreProTask < Data > PrePro;
  typedef ucam::hifst::PatternsToInstancesTask < Data > PatternsToInstances;
  typedef ucam::hifst::SentenceSpecificGrammarTask < Data >
  SentenceSpecificGrammar;

  ucam::util::FastForwardRead<ucam::util::iszfstream> fastforwardread_;
  const ucam::util::RegistryPO& rg_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to ucam::util::RegistryPO object with all parsed parameters.
   */
  SingleThreadedCreateSentenceSpecificGrammarTask ( const ucam::util::RegistryPO
      &rg ) :
    fastforwardread_ ( new ucam::util::iszfstream ( rg.get<std::string>
                       ( HifstConstants::kSourceLoad ) ) ),
    rg_ ( rg ) {
  };

  /**
   * \brief Reads an input sentence, tokenizes and integer-maps.
   * \param d : data object in which models, fsts, etc are stored and passed through to several tasks
   */
  bool run ( Data& d ) {
    boost::scoped_ptr < LoadGrammar> grammartask ( new LoadGrammar ( rg_ ) );
    grammartask->appendTask
    ( LoadWordMap::init ( rg_  , HifstConstants::kPreproWordmapLoad , true ) )
    ( new PrePro ( rg_ ) )
    ( new PatternsToInstances ( rg_ ) )
    ( new SentenceSpecificGrammar ( rg_ ) )
    ;
    bool finished = false;
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ) );
          !ir->done ();
          ir->next () ) {
      d.sidx = ir->get ();
      finished = fastforwardread_ ( d.sidx ,
                                    &d.originalsentence ); //Move to whichever next sentence and read
      FORCELINFO ( "=====Translate sentence " << d.sidx << ":" <<
                   d.originalsentence );
      grammartask->chainrun ( d );        //Run translation!
      if ( finished ) break;
    }
    return false;
  };

  inline bool operator() () {
    Data d;
    return run ( d );
  }

 private:

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadedCreateSentenceSpecificGrammarTask );

};

/**
 * \brief Full multi-threaded Translation system
 */

template <class Data = HifstTaskData>
class MultiThreadedCreateSentenceSpecificGrammarTask: public
  ucam::util::TaskInterface<Data> {

 private:
  typedef ucam::hifst::GrammarTask < Data > LoadGrammar;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::hifst::PreProTask < Data > PrePro;
  typedef ucam::hifst::PatternsToInstancesTask < Data > PatternsToInstances;
  typedef ucam::hifst::SentenceSpecificGrammarTask < Data >
  SentenceSpecificGrammar;

  ucam::util::FastForwardRead<ucam::util::iszfstream> fastforwardread_;
  const ucam::util::RegistryPO& rg_;
  unsigned threadcount_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to ucam::util::RegistryPO object with all parsed parameters.
   */
  MultiThreadedCreateSentenceSpecificGrammarTask ( const ucam::util::RegistryPO
      &rg ) :
    fastforwardread_ ( new ucam::util::iszfstream ( rg.get<std::string>
                       ( HifstConstants::kSourceLoad ) ) ),
    threadcount_ ( rg.get<unsigned> ( HifstConstants::kNThreads) ),
    rg_ ( rg ) {
  };

  /**
   * \brief Reads an input sentence, tokenizes and integer-maps.
   * \param original_data : data object in which models, fsts, etc are stored and passed through to several tasks
   */
  bool run ( Data& original_data ) {
    boost::scoped_ptr < LoadGrammar >grammartask ( new LoadGrammar ( rg_ ) );
    grammartask->appendTask
    ( LoadWordMap::init ( rg_  , HifstConstants::kPreproWordmapLoad , true ) )
    ;
    //Load grammar and language model
    grammartask->chainrun ( original_data );
    std::vector < boost::shared_ptr<std::string> >translations;
    {
      ucam::util::TrivialThreadPool tp ( threadcount_ );
      bool finished = false;
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                        HifstConstants::kRangeOne ) );
            !ir->done();
            ir->next() ) {
        Data *d = new Data; //( original_data ); // reset.
        d->grammar = original_data.grammar;
        d->sidx = ir->get();
        d->wm = original_data.wm;
        finished = fastforwardread_ ( d->sidx ,
                                      & ( d->originalsentence ) ); //Move to whichever next sentence and read
        FORCELINFO ( "=====Translate sentence " << d->sidx << ":" <<
                     d->originalsentence );
        PrePro *p = new PrePro ( rg_ );
        p->appendTask
        ( new PatternsToInstances ( rg_ ) )
        ( new SentenceSpecificGrammar ( rg_ ) )
        ;
        tp ( ucam::util::TaskFunctor<Data> ( p, d ) );
        if ( finished ) break;
      }
    }
    return false;
  };

  ///Run using its own internal data object
  inline bool operator() () {
    Data d;
    return run ( d );
  }

 private:

  DISALLOW_COPY_AND_ASSIGN ( MultiThreadedCreateSentenceSpecificGrammarTask );

};

}
}  // end namespaces

#endif
