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

#ifndef MAIN_RUN_HIFST_HPP
#define MAIN_RUN_HIFST_HPP

/**
 * \file
 * \brief Contains hifst core: implements single threaded, multithreaded or as server
 * \date 1-10-2012
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
template < template <class> class DataT
           , class ArcT
           >
class SingleThreadedHifstTask: public ucam::util::TaskInterface<DataT<ArcT>  > {
 private:
  typedef DataT<ArcT> Data;
  typedef ucam::hifst::GrammarTask < Data > LoadGrammar;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::LoadUnimapTask< Data , ArcT > LoadUnimap;
  typedef ucam::hifst::PreProTask < Data > PrePro;
  typedef ucam::fsttools::LoadLanguageModelTask < Data> LoadLanguageModel;
  typedef ucam::hifst::PatternsToInstancesTask < Data > PatternsToInstances;
  typedef ucam::hifst::ReferenceFilterTask < Data , ArcT > ReferenceFilter;
  typedef ucam::hifst::SentenceSpecificGrammarTask < Data > SentenceSpecificGrammar;
  typedef ucam::hifst::CYKParserTask < Data > Parse;
  typedef ucam::hifst::HiFSTTask < Data,  ArcT > HiFST;
  typedef ucam::fsttools::OptimizeFstTask< Data , ArcT > OptimizeFst;
  typedef ucam::fsttools::WriteFstTask< Data , ArcT > WriteFst;
  typedef ucam::fsttools::DisambigTask< Data, ArcT > Recase;
  typedef ucam::hifst::PostProTask < Data , ArcT > PostPro;
  typedef ucam::hifst::HifstStatsTask < Data > HifstStats;
  typedef ucam::util::iszfstream iszfstream;
  typedef ucam::util::oszfstream oszfstream;

  ///Reads appropriate sentences from file according to values provided by range
  ucam::util::FastForwardRead<iszfstream> fastforwardread_;
  ///Name of file translations are written into.
  std::string textoutput_;
  ///Registry object
  const ucam::util::RegistryPO& rg_;
  bool usingTupleArc_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to RegistryPO object with all parsed parameters.
   */
  SingleThreadedHifstTask ( const ucam::util::RegistryPO& rg )
      : fastforwardread_ ( new iszfstream ( rg.get<std::string>
                                            ( HifstConstants::kSourceLoad ) ) )
      , textoutput_ ( rg.get<std::string> ( HifstConstants::kTargetStore ) )
      , rg_ ( rg )
      , usingTupleArc_(rg.get<std::string>(HifstConstants::kHifstSemiring) == HifstConstants::kHifstSemiringTupleArc )
  {
    if (!usingTupleArc_) return;
    // If we are using the tropical sparse tuple semiring, we need to set feature weights;
    // Artificial parameters are:
    // language model(s) feature weights: use the ones provided by the user.
    // single grammar feature (dot product), hence dot product feature weight=1.
    // local language models (dot product), hence dot product feature weight=1.
    std::string params = rg.get<std::string>(HifstConstants::kLmFeatureweights);
    params += ",1,1";
    FORCELINFO("fake tuple params=" << params);
    TupleW32::Params() = ucam::util::ParseParamString<float> (params);
  };

  /**
   * \brief Translates an input sentence (single threaded)
   * \param d     Data object that communicates all the tasks.
   */
  bool run ( Data& d ) {
    using namespace HifstConstants;
    // Assigns keys to fetch feature weights appropriately
    // This keeps all current options available
    // User can work with lm.featureweights + grammar.featureweights
    // or only featureweights, as a concatenation of the two previous
    // (language model features first)
    // Probably lm.featureweights and grammar.featureweights will be deprecated
    // at some point.
    std::string lmFeatureweights = kLmFeatureweights;
    std::string grammarFeatureweights = kGrammarFeatureweights;
    unsigned grammarFeatureweightOffset = 0;
    if (rg_.get<std::string> (kFeatureweights) != "") {
      grammarFeatureweights = lmFeatureweights = kFeatureweights;
      grammarFeatureweightOffset = rg_.getVectorString (
                                     kLmLoad).size();
    }
    boost::scoped_ptr < LoadGrammar> grammartask
        ( new LoadGrammar ( rg_, grammarFeatureweights, grammarFeatureweightOffset ) );
    grammartask->appendTask
        ( LoadWordMap::init ( rg_  , kPreproWordmapLoad , true ) )
        ( LoadWordMap::init ( rg_  , kPostproWordmapLoad ) )
    //TODO: add an option to ensure that postpro.wordmap uses this one
    //, but then reversed wordmap is forced
        ( LoadWordMap::init ( rg_  , kLmWordmap , true) )
        ( new LoadUnimap ( rg_  , kRecaserUnimapLoad ) )
        ( new PrePro ( rg_ ) )
        ( new LoadLanguageModel ( rg_
                                  , kLmLoad
                                  , (usingTupleArc_)? "":lmFeatureweights ) )
        ( new LoadLanguageModel ( rg_
                                  , kHifstLocalpruneLmLoad
                                  , kHifstLocalpruneLmFeatureweights
                                  , kHifstLocalpruneLmWordpenalty) )
        ( new LoadLanguageModel ( rg_
                                  , kRecaserLmLoad
                                  , kRecaserLmFeatureweight
                                  , kRecaserLmWps
                                  , kRecaserLmWordmap ) )
        ( new PatternsToInstances ( rg_ ) )
        ( ReferenceFilter::init ( rg_ ) )
        ( new SentenceSpecificGrammar ( rg_ ) )
        ( new Parse ( rg_ ) )
        ( new HiFST ( rg_ ) )
        ( OptimizeFst::init(rg_, kHifstLatticeOptimize, kHifstLatticeStore , kHifstStripSpecialEpsilonLabels) )
        ( WriteFst::init ( rg_, kHifstLatticeStore )  )
        ( new Recase ( rg_
                     , kHifstLatticeStore
                     , kPostproInput
                     , kRecaserLmLoad ) )
        ( WriteFst::init ( rg_ , kRecaserOutput, kPostproInput ) )
        ( new PostPro ( rg_ ) )
        ( new HifstStats ( rg_ ) )
        ;
    bool finished = false;
    oszfstream *fileoutput = NULL;
    if ( textoutput_ != "" ) {
      fileoutput = new oszfstream ( textoutput_ );
    }
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ) );
          !ir->done ();
          ir->next () ) {
      d.sidx = ir->get ();
      d.filters.clear();
      boost::scoped_ptr<std::string> aux ( new std::string ( "" ) );
      d.translation = aux.get();
      //Move to whichever next sentence and read
      finished = fastforwardread_ ( d.sidx , &d.originalsentence );
      boost::algorithm::trim (d.originalsentence);
      if (finished && d.originalsentence == "" ) break;
      FORCELINFO ( "=====Translate sentence " << d.sidx << ":" <<
                   d.originalsentence );
      grammartask->chainrun ( d );        //Run translation!
      if ( fileoutput != NULL )
        *fileoutput << *d.translation << endl;
      if ( finished ) break;
    }
    if ( fileoutput != NULL )
      delete fileoutput;
    return false;
  };

  ///Runs using its own internal data object
  inline bool operator() () {
    Data d;
    return run ( d );
  }

 private:

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadedHifstTask );

};

/**
 * \brief Full multi-threaded Translation system
 */

template < template <class> class DataT
           , class ArcT
           >
class MultiThreadedHifstTask: public ucam::util::TaskInterface<DataT<ArcT> > {

 private:
  typedef DataT<ArcT> Data;
  typedef ucam::hifst::GrammarTask < Data > LoadGrammar;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::LoadUnimapTask< Data , ArcT > LoadUnimap;
  typedef ucam::hifst::PreProTask < Data > PrePro;
  typedef ucam::fsttools::LoadLanguageModelTask < Data > LoadLanguageModel;
  typedef ucam::hifst::PatternsToInstancesTask < Data > PatternsToInstances;
  typedef ucam::hifst::ReferenceFilterTask < Data , ArcT > ReferenceFilter;
  typedef ucam::hifst::SentenceSpecificGrammarTask < Data > SentenceSpecificGrammar;
  typedef ucam::hifst::CYKParserTask < Data > Parse;
  typedef ucam::hifst::HiFSTTask < Data, ArcT > HiFST;
  typedef ucam::fsttools::OptimizeFstTask< Data , ArcT > OptimizeFst;
  typedef ucam::fsttools::WriteFstTask< Data , ArcT > WriteFst;
  typedef ucam::fsttools::DisambigTask< Data, ArcT > Recase;
  typedef ucam::hifst::PostProTask < Data , ArcT > PostPro;
  typedef ucam::hifst::HifstStatsTask < Data > HifstStats;
  typedef ucam::util::iszfstream iszfstream;
  typedef ucam::util::oszfstream oszfstream;

  ///Reads appropriate sentences from file according to values provided by range
  ucam::util::FastForwardRead<iszfstream> fastforwardread_;

  ///file name -- where to output translation
  std::string textoutput_;

  ///Registry object
  const ucam::util::RegistryPO& rg_;

  ///Number of threads requested by user
  unsigned threadcount_;
  bool usingTupleArc_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to ucam::util::RegistryPO object with all parsed parameters.
   */
  MultiThreadedHifstTask ( const ucam::util::RegistryPO& rg )
      : fastforwardread_ ( new iszfstream ( rg.get<std::string>
                                        ( HifstConstants::kSourceLoad ) ) )
      , textoutput_ ( rg.get<std::string> ( HifstConstants::kTargetStore ) )
      , threadcount_ ( rg.get<unsigned> ( HifstConstants::kNThreads ) )
      , usingTupleArc_(rg.get<std::string>(HifstConstants::kHifstSemiring) == HifstConstants::kHifstSemiringTupleArc )
      , rg_ ( rg ) {

    if (!usingTupleArc_) return;
    // If we are using the tropical sparse tuple semiring, we need to set feature weights;
    // Artificial parameters are:
    // language model(s) feature weights: use the ones provided by the user.
    // single grammar feature (dot product), hence dot product feature weight=1.
    // local language models (dot product), hence dot product feature weight=1.
    std::string params = rg.get<std::string>(HifstConstants::kLmFeatureweights);
    params += ",1,1";
    FORCELINFO("fake tuple params=" << params);
    TupleW32::Params() = ucam::util::ParseParamString<float> (params);
  };

  /**
   * \brief Translates an input sentence (multithreaded)
   * \param original_data     Data object that communicates all the tasks.
   */

  bool run ( Data& original_data ) {
    using namespace HifstConstants;

    std::string lmFeatureweights = kLmFeatureweights;
    std::string grammarFeatureweights = kGrammarFeatureweights;
    unsigned grammarFeatureweightOffset = 0;
    if (rg_.get<std::string> (kFeatureweights) != "") {
      grammarFeatureweights = lmFeatureweights = kFeatureweights;
      grammarFeatureweightOffset = rg_.getVectorString (
                                     kLmLoad).size();
    }
    boost::scoped_ptr < LoadGrammar > grammartask
    ( new LoadGrammar ( rg_, grammarFeatureweights, grammarFeatureweightOffset ) );
    grammartask->appendTask
    ( new LoadLanguageModel ( rg_
                              , kLmLoad
                              , lmFeatureweights ) )
    ( new LoadLanguageModel ( rg_
                              , kHifstLocalpruneLmLoad
                              , kHifstLocalpruneLmFeatureweights
                              , kHifstLocalpruneLmWordpenalty) )
    ( new LoadLanguageModel ( rg_
                              , kRecaserLmLoad
                              , kRecaserLmFeatureweight
                              , kRecaserLmWps
                              , kRecaserLmWordmap) )
    ( new LoadUnimap ( rg_  , kRecaserUnimapLoad ) )
    ( LoadWordMap::init ( rg_  , kPreproWordmapLoad , true ) )
    ( LoadWordMap::init ( rg_  , kPostproWordmapLoad ) )
    ;
    //Load grammar and language model
    grammartask->chainrun ( original_data );
    std::vector < boost::shared_ptr<std::string> >translations;
    {
      ucam::util::TrivialThreadPool tp ( threadcount_ );
      bool finished = false;
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ) );
            !ir->done();
            ir->next() ) {
        Data *d = new Data; //( original_data ); // reset.
        d->grammar = original_data.grammar;
        d->sidx = ir->get();
        d->klm = original_data.klm;
        translations.push_back ( boost::shared_ptr<std::string>
                                 ( new std::string ( "" ) ) );
        d->translation = translations[translations.size() - 1].get();
        if ( original_data.fsts.find ( kRecaserUnimapLoad ) !=
             original_data.fsts.end() )
          d->fsts[kRecaserUnimapLoad] =
            original_data.fsts[kRecaserUnimapLoad];
        d->recasingvcblm = original_data.recasingvcblm;
        d->wm = original_data.wm;
        finished = fastforwardread_ ( d->sidx ,
                                      & ( d->originalsentence ) ); //Move to whichever next sentence and read
        if (finished && d->originalsentence == "") break;
        FORCELINFO ( "=====Translate sentence " << d->sidx << ":" <<
                     d->originalsentence );
        PrePro *p = new PrePro ( rg_ );
        p->appendTask
        ( new PatternsToInstances ( rg_ ) )
        ( ReferenceFilter::init ( rg_ ) )
        ( new SentenceSpecificGrammar ( rg_ ) )
        ( new Parse ( rg_ ) )
        ( new HiFST ( rg_ ) )
        ( OptimizeFst::init(rg_, kHifstLatticeOptimize, kHifstLatticeStore , kHifstStripSpecialEpsilonLabels) )
        ( WriteFst::init ( rg_, kHifstLatticeStore )  )
        ( new Recase ( rg_  ,
                       kHifstLatticeStore,
                       kPostproInput,
                       kRecaserLmLoad,
                       kRecaserUnimapLoad ) )
        ( WriteFst::init ( rg_ , kRecaserOutput, kPostproInput ) )
        ( new PostPro ( rg_ ) )
        ( new HifstStats ( rg_ ) )
        ;
        tp ( ucam::util::TaskFunctor<Data> ( p, d ) );
        if ( finished ) break;
      }
    }
    ///Todo here... Traverse translations and write text output
    if ( textoutput_ == "" ) return false;
    boost::scoped_ptr<oszfstream> fileoutput ( new oszfstream ( textoutput_ ) );
    for ( unsigned k = 0; k < translations.size(); ++k )
      *fileoutput << *translations[k] << endl;
    return false;
  };

  ///Runs translation with an internal data object.
  inline bool operator() () {
    Data d;
    return run ( d );
  }

 private:

  DISALLOW_COPY_AND_ASSIGN ( MultiThreadedHifstTask );

};

/**
 * \brief Translation Server
 */

template < template <class> class DataT
           , class ArcT
           >
class HifstServerTask: public ucam::util::TaskInterface<DataT<ArcT>  > {
 private:
  typedef DataT<ArcT> Data;
  typedef ucam::hifst::GrammarTask < Data > LoadGrammar;
  typedef ucam::fsttools::LoadWordMapTask< Data > LoadWordMap;
  typedef ucam::fsttools::LoadUnimapTask< Data , ArcT > LoadUnimap;
  typedef ucam::hifst::PreProTask < Data > PrePro;
  typedef ucam::fsttools::LoadLanguageModelTask < Data > LoadLanguageModel;
  typedef ucam::hifst::PatternsToInstancesTask < Data > PatternsToInstances;
  typedef ucam::hifst::ReferenceFilterTask < Data , ArcT > ReferenceFilter;
  typedef ucam::hifst::SentenceSpecificGrammarTask < Data > SentenceSpecificGrammar;
  typedef ucam::hifst::CYKParserTask < Data > Parse;
  typedef ucam::hifst::HiFSTTask < Data, ArcT > HiFST;
  typedef ucam::fsttools::WriteFstTask< Data , ArcT > WriteFst;
  typedef ucam::fsttools::DisambigTask< Data, ArcT > Recase;
  typedef ucam::hifst::PostProTask < Data , ArcT > PostPro;
  typedef ucam::hifst::HifstStatsTask < Data > HifstStats;

  ///Registry object
  const ucam::util::RegistryPO& rg_;

  ///Port at which hifst is listening
  short port_;

  ///Data object
  Data d_;

  ///Translation task
  boost::scoped_ptr < GrammarTask < Data > >ttask_;

  ///Core internal class doing the actual translation
  class translation {
   private:
    ///Registry object
    const ucam::util::RegistryPO& rg_;

   public:
    ///constructor
    translation ( const ucam::util::RegistryPO& rg ) : rg_ ( rg ) {};

    /**
     * \brief Assuming a request has arrived, it generates the translation and sends it back to the client.
     * Reads source sentence size (bytes) and sentence itself, translates and then writes translation size
     * and translation itself into the socket.
     * \param sock      Socket object
     * \param d         Data object
     */
    bool operator () ( socket_ptr sock, Data& d ) {
      using namespace HifstConstants;
      ///Make a copy of d, and go for it.
      LINFO ( "Init new taskdata..." );
      boost::scoped_ptr<Data> mydata ( new Data );
      mydata->grammar = d.grammar;
      mydata->klm = d.klm;
      mydata->filters.clear();
      if ( d.fsts.find ( kRecaserUnimapLoad ) != d.fsts.end() )
        mydata->fsts[kRecaserUnimapLoad] =
          d.fsts[kRecaserUnimapLoad];
      mydata->recasingvcblm = d.recasingvcblm;
      mydata->wm = d.wm;
      LINFO ("Number of wordmaps... " << mydata->wm.size() );
      try {
        char data[max_length + 1];
        std::size_t query_length = 0;
        std::size_t query_length1 = boost::asio::read ( *sock,
                                    boost::asio::buffer ( &query_length, sizeof ( std::size_t ) ) );
        std::size_t query_length2 = boost::asio::read ( *sock,
                                    boost::asio::buffer ( data, query_length ) );
        data[query_length2] = 0;
        mydata->originalsentence = data;
        FORCELINFO ( "Query to translate: " << mydata->originalsentence );
        boost::scoped_ptr<std::string> translation ( new std::string );
        mydata->translation = translation.get();
        this->operator() ( *mydata );
        char datasend[max_length + 1];
        //Send data
        strcpy ( datasend, ( char * ) translation.get()->c_str() );
        std::size_t length = strlen ( datasend );
        boost::asio::write ( *sock, boost::asio::buffer ( &length,
                             sizeof ( std::size_t ) ) );
        FORCELINFO ( "Sending:" << datasend );
        boost::asio::write ( *sock, boost::asio::buffer ( datasend, length ) );
        sock->close();
      } catch ( std::exception& e ) {
        std::cerr << "Exception in thread! " << e.what() << "\n";
      }
      return true;
    };

   private:

    /**
     * \brief Runs actual translation tasks using models in data object
     */
    bool operator () ( Data& d ) {
      using namespace HifstConstants;
      PrePro p ( rg_ );
      p.appendTask
      ( new PatternsToInstances ( rg_ ) )
      ( new SentenceSpecificGrammar ( rg_ ) )
      ( new Parse ( rg_ ) )
      ( new HiFST ( rg_ ) )
      ( new WriteFst ( rg_  , kHifstLatticeStore )  )
      ( new Recase ( rg_  ,
                     kHifstLatticeStore,
                     kPostproInput,
                     kRecaserLmLoad,
                     kRecaserUnimapLoad ) )
      ( new PostPro ( rg_ ) )
      ;
      p.chainrun ( d );        //Run translation!
    }

  };

 public:
  /**
   *\brief Constructor
   *\param rg: RegistryPO object with all parsed parameters.
   */
  HifstServerTask ( const ucam::util::RegistryPO& rg ) :
    rg_ ( rg ),
    port_ ( rg.get<short> ( HifstConstants::kServerPort ) ) {
  };

  ///Loads all full models once (grammar, language models, wordmap files).
  ///Pointers to these models will be stored in internal data object d_.
  void load() {
    using namespace HifstConstants;
    std::string lmFeatureweights = kLmFeatureweights;
    std::string grammarFeatureweights = kGrammarFeatureweights;
    unsigned grammarFeatureweightOffset = 0;
    if (rg_.get<std::string> (kFeatureweights) != "") {
      grammarFeatureweights = lmFeatureweights = kFeatureweights;
      grammarFeatureweightOffset = rg_.getVectorString (kLmLoad).size();
    }
    ttask_.reset ( new LoadGrammar ( rg_, grammarFeatureweights,
                                     grammarFeatureweightOffset ) );
    ttask_->appendTask
    ( new LoadLanguageModel ( rg_
                              , kLmLoad
                              , lmFeatureweights ) )
    ( new LoadLanguageModel ( rg_
                              , kHifstLocalpruneLmLoad
                              , kHifstLocalpruneLmFeatureweights
                              , kHifstLocalpruneLmWordpenalty
                            ) )
    ( new LoadLanguageModel ( rg_
                              , kRecaserLmLoad
                              , kRecaserLmFeatureweight
                            ) )
    ( new LoadUnimap ( rg_  , kRecaserUnimapLoad ) )
    ( LoadWordMap::init ( rg_  , kPreproWordmapLoad, true ) )
    ( LoadWordMap::init ( rg_  , kPostproWordmapLoad ) )
    ;
    //Load grammar and language model
    ttask_->chainrun ( d_ );
  };

  ///Loads and starts translation system using internal data object containing pointers to loaded models
  inline bool operator() () {
    load();
    return run ( d_ );
  }

 private:

  ///Kicks off server, waits for incoming request, when it arrives
  ///it generates a thread to attend request and keeps listening for more requests.
  bool run ( Data& d ) {
    boost::asio::io_service io_service;
    tcp::acceptor a ( io_service, tcp::endpoint ( tcp::v4(), port_ ) );
    for ( ;; ) {
      LINFO ( "Waiting for a connection at port=" << port_ );
      socket_ptr sock ( new tcp::socket ( io_service ) );
      a.accept ( *sock );
      translation tr ( rg_ );
      boost::thread t ( boost::bind<void> ( tr, sock, d ) );
      LINFO ( "Connection accepted... Thread created..." );
    }
  };

  DISALLOW_COPY_AND_ASSIGN ( HifstServerTask );

};

}
}  // end namespaces

#endif // MAIN_RUN_HIFST_HPP
