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

#ifndef LOADLMTASK_HPP
#define LOADLMTASK_HPP

/**
 * \file
 * \brief Implementation of a language model task
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <kenlmdetect.hpp>
#include <lm/config.hh>
#include <lm/enumerate_vocab.hh>
#ifdef WITH_NPLM
#include <lm/wrappers/nplm.hh>
#endif
#include <idbridge.hpp>
#include <hifst_enumerate_vocab.hpp>

namespace ucam {
namespace fsttools {

// External wrapped-in implementations
// don't necessarily have the same constructor.
// This class handles the general case
// and template specialization helps to handle exceptions
template<class KenLMModelT> 
struct KenLMModelHelper {
  std::string const file_;
  lm::ngram::Config &kenlm_config_;
  KenLMModelHelper(std::string const &file
		   , lm::ngram::Config kenlm_config)
    : file_(file)
    , kenlm_config_(kenlm_config)
  {}
    
  KenLMModelT *operator()(){
    return new KenLMModelT ( file_.c_str() , kenlm_config_);
  }

};

#ifdef WITH_NPLM
// Specialization for NPLM:
template<>
struct KenLMModelHelper<lm::np::Model> {
  std::string const file_;
  lm::ngram::Config &kenlm_config_;
  KenLMModelHelper(std::string const &file
		   , lm::ngram::Config kenlm_config)
    : file_(file)
    , kenlm_config_(kenlm_config)
  {}
  lm::np::Model *operator()(){
    return new lm::np::Model( file_);
  }
};
#endif

lm::base::Model *loadKenLm(std::string const &file
			   , lm::ngram::Config kenlm_config 
			   , unsigned offset = 0) {
  using namespace lm::ngram;				     
  typedef lm::np::Model NplmModel;
  // Detect here kenlm binary type
  int  kenmt = ucam::util::detectkenlm(file);
  switch (kenmt) {
  case PROBING:
    return KenLMModelHelper<ProbingModel>(file, kenlm_config)();
  case REST_PROBING:
    return KenLMModelHelper<RestProbingModel>(file, kenlm_config)();
  case TRIE:
    return KenLMModelHelper<TrieModel>(file, kenlm_config)();
 case QUANT_TRIE:
    return KenLMModelHelper<QuantTrieModel>(file, kenlm_config)();
 case ARRAY_TRIE:
    return KenLMModelHelper<ArrayTrieModel>(file, kenlm_config)();
 case QUANT_ARRAY_TRIE:
    return KenLMModelHelper<QuantArrayTrieModel>(file, kenlm_config)();
 case util::KENLM_NPLM:
#ifdef WITH_NPLM
    return KenLMModelHelper<NplmModel>(file, kenlm_config)();
#endif
    LERROR("Unsuported format: KENLM_NPLM. Did you compile NPLM library?");
    exit(EXIT_FAILURE);
  }
  return NULL;
};





/**
 * \brief Language model loader task, loads a language model wrapping it in a class to provide.
 *
 */

template <class Data>
class LoadLanguageModelTask: public ucam::util::TaskInterface<Data> {
  
  typedef lm::base::Model KenLMModelT;

 private:

  ///Built or not
  bool built_;

  ///Language model file (possibly sentence-specific)
  ucam::util::IntegerPatternAddress lmfile_;

  ///Previous language model file
  std::string previous_;
  uint lmo_;

  ///Data object for kenlm
  KenLMData kld_;

  ///Index of the actual language model, if several were created.
  uint index_;
  ///key to store the language models
  std::string lmkey_;

  const ucam::util::RegistryPO& rg_;

  const std::string wordmapkey_;
  bool isintegermapped_;

 public:

  /**
   * \brief Public constructor. If the user wants to load several language models  (e.g. --lm.load=lm1,lm2,lm3,lm4 and --lm.scale=0.25,0.25,0.25 ),
   * the second and following instances of LoadLanguageModelTask will be created using the private constructor (see below), which has an index to the actual language model that must be loaded.
   * For the public constructor, the index is set to 0.
   * \param rg        ucam::util::RegistryPO object, containing user parameters.
   * \param lmload    key word to access the registry object for language models
   * \param lmscale   key word to access the registry object for language model scales.
   * \param forceone  To force the loading of only one language model (i.e. lm1 with scale 0.25).
   */
  LoadLanguageModelTask ( const ucam::util::RegistryPO& rg ,
                          const std::string& lmload = HifstConstants::kLmLoad ,
                          const std::string& lmscale =
                            HifstConstants::kLmFeatureweights,  //if rg.get(lmscale)=="" the scale will default to 1
                          const std::string& lmwp =
                            HifstConstants::kLmWordPenalty,  //if rg.get(wps)=="" the scale will default to 0
                          const std::string& wordmapkey = HifstConstants::kLmWordmap,
                          bool forceone = false
                        ) :
    rg_ ( rg ),
    lmkey_ ( lmload ),
    previous_ ( "" ),
    built_ ( false ),
    index_ ( 0 ),
    isintegermapped_ (!rg.exists (wordmapkey)
                      || rg.get<std::string> (wordmapkey) == ""),
    wordmapkey_ (wordmapkey),
    lmfile_ ( rg.getVectorString ( lmload , 0 ) ) {
    LINFO ( "LM loader using parameters " << lmload << "/" << lmscale << "/" << lmwp
            << ", and key " << lmkey_  << ",index=" << index_ << ",wordmap=" <<
            wordmapkey_);
    setLanguageModelScale ( lmscale );
    setLanguageModelWordPenalty ( lmwp );
    if ( rg_.getVectorString ( lmload ).size() > 1 ) {
      if ( !forceone ) {
        LINFO ( "Appending Language model..." );
        this->appendTask ( new LoadLanguageModelTask ( rg_, 1, lmload, lmscale , lmwp ,
                           wordmapkey ) );
      } else {
        LWARN ( "Only one loaded for " << lmload <<
                ". Extra language models are being ignored" );
      }
    }
    LINFO ( "Finished constructor!" );
  };

  /**
   * \brief Method inherited from TaskInterface. Loads the language model and stores in lm data structure.
   * \param &d: data structure in which the null filter is to be stored.
   * \returns false (does not break in any case the chain of tasks)
   */
  bool run ( Data& d ) {
    LINFO ( "run!" );
    if ( lmfile_() == "" ) return false;
    // No need to build again...
    if ( built_ && previous_ == lmfile_ ( d.sidx ) ) return false;
    close();
    FORCELINFO ( "loading LM=" << lmfile_ ( d.sidx ) );
    d.stats->setTimeStart ("lm-load-" + index_ );
    lm::ngram::Config kenlm_config;
    // If lm is not integermapped, then we will need a proper grammar target wordmap.
    // Make sure we have it.
    ucam::util::WordMapper *wm = NULL;
    if (!isintegermapped_) {
      LINFO ("Using wordmap " << wordmapkey_);
      LINFO ("There are " << d.wm.size() << " wordmaps");
      USER_CHECK (d.wm.find (wordmapkey_) != d.wm.end()
                  , "Language model provided over words instead of integers. A target wordmap is required! ");
      wm = d.wm[wordmapkey_];
    }
    lm::HifstEnumerateVocab hev (kld_.idb, wm);
    kenlm_config.enumerate_vocab = &hev;
    kld_.model = loadKenLm(lmfile_(d.sidx).c_str(), kenlm_config, index_);
    d.stats->setTimeEnd ("lm-load-" + index_ );
    previous_ = lmfile_ ( d.sidx );
    built_ = true;
    if ( d.klm.find ( lmkey_ ) == d.klm.end() ) d.klm[lmkey_].resize ( index_ + 1 );
    else if ( d.klm[lmkey_].size() < index_ + 1 ) d.klm[lmkey_].resize (
        index_ + 1 );
    d.klm[lmkey_][index_] = (const KenLMData*) &kld_;
    LDEBUG ( "LM " << lmfile_ ( d.sidx ) << " loaded, key=" << lmkey_ <<
             ", position=" <<  toString<uint> ( d.klm[lmkey_].size() - 1 ) <<
             ",total number of language models for this key is " << d.klm[lmkey_].size() );
    return false;
  };

  /// Free language model resources. Returns true if ok, false if otherwise.
  bool close() {
    if ( kld_.model != NULL ) {
      LINFO ( "Releasing language model resources..." );
      delete kld_.model;
      kld_.model = NULL;
      built_ = false;
      return true;
    }
    return false;
  }

  ///Destructor
  ~LoadLanguageModelTask() {
    close();
  }

 private:

  /**
   * \brief Private constructor with ucam::util::RegistryPO object and index to a particular language model.
   * This constructor is only used when several language models are loaded.
   * If the user wants to load several language models  (e.g. --lm.load=lm1,lm2,lm3,lm4 and --lm.scale=0.25,0.25,0.25 ).
   * The second and following instances of LoadLanguageModelTask will be created using the private constructor.
   * In the constructor itself the next language model loader task is appended.
   * this constructor, which has an index to the actual language model that must be loaded.
   * \param rg      :  ucam::util::RegistryPO object, containing user parameters.
   * \param index   :  Index to the actual language model.
   * \param lmload  :  key word to access the registry object for language models
   * \param lmscale :  key word to access the registry object for language model scales.
   */
  LoadLanguageModelTask ( const ucam::util::RegistryPO& rg ,
                          uint index ,
                          const std::string& lmload = HifstConstants::kLmLoad,
                          const std::string& lmscale = HifstConstants::kLmFeatureweights ,
                          const std::string& lmwp = HifstConstants::kLmWordPenalty,
                          const std::string& wordmapkey = HifstConstants::kLmWordmap
                        ) :
    rg_ ( rg ),
    lmkey_ ( lmload ),
    previous_ ( "" ),
    built_ ( false ),
    index_ ( index ),
    isintegermapped_ (!rg.exists (wordmapkey)
                      || rg.get<std::string> (wordmapkey) == ""),
    wordmapkey_ (wordmapkey),
    lmfile_ ( rg.getVectorString ( lmload , index ) ) {
    LINFO ( "LM loader using parameters " << lmload << "/" << lmscale <<
            ", and key " << lmkey_  << ",index=" << index_ << ",wordmapkey=" <<
            wordmapkey_);
    setLanguageModelScale ( lmscale );
    setLanguageModelWordPenalty ( lmwp );
    if ( rg.getVectorString ( lmload ).size() > index_ + 1 ) {
      LINFO ( "Appending Language model..." );
      this->appendTask ( new LoadLanguageModelTask ( rg, index_ + 1, lmload, lmscale ,
                         lmwp , wordmapkey ) );
    }
    LINFO ( "Ready!" );
  };

  /**
   * \brief Sets language model scales using a key to obtain these scales from ucam::util::RegistryPO object.
   * If this key is empty, will default to 1.0f.
   *  \param lmscale The key to access the language model scale in the ucam::util::RegistryPO object.
   */

  void setLanguageModelScale ( const std::string& lmscale ) {
    kld_.lmscale = 1.0f;
    if (!rg_.exists (lmscale) ) {
      FORCELINFO ( "Language model scale " << index_  << " defaulting to 1.0f" );
      return;
    }
    if (rg_.get<std::string> (lmscale) == "" ) {
      FORCELINFO ( "Language model scale " << index_  << " defaulting to 1.0f" );
      return;
    }
    std::string aux = rg_.getVectorString ( lmscale, index_ );
    kld_.lmscale = ucam::util::toNumber<float> ( aux );
    FORCELINFO ("Language model scale " << index_ << "=" << aux );
  }

  /**
   * \brief Sets language model word penalties using a key to obtain these scales from ucam::util::RegistryPO object.
   * If this key is empty, will default to 0.0f.
   * \remark Word penalties are typically inserted in hifst as part of the grammar.
   * However, a tool such as hipdt requires language model application with lm-specific word penalties
   * -- corrections back and forth around the word penalty used with the grammar.
   *  \param lmwp The key to access the language model scale in the ucam::util::RegistryPO object.
   */

  void setLanguageModelWordPenalty ( const std::string& lmwp ) {
    kld_.lmwp = 0.0f;
    if (!rg_.exists (lmwp) ) {
      FORCELINFO ( "Language model word penalty " << index_  <<
                   " defaulting to 0.0f" );
      return;
    }
    if (rg_.get<std::string> (lmwp) == "" ) {
      FORCELINFO ( "Language model scale " << index_  << " defaulting to 0.0f" );
      return;
    }
    std::string aux = rg_.getVectorString ( lmwp, index_ );
    kld_.lmwp = ucam::util::toNumber<float> ( aux );
    FORCELINFO ( "Language model word penalty " << index_  <<  "=" << aux );
  }
};

}
} // end namespaces
#endif
