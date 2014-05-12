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

#ifndef TASK_PREPRO_HPP
#define TASK_PREPRO_HPP

/**
 * \file
 * \brief Describes class PreProTask, which preprocesses (tokenizes and maps to integers with WordMapper) source input
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Reads text file, performs tokenization and integer-mapping.
 */

template <class Data>
class PreProTask: public ucam::util::TaskInterface<Data> {

  //Private variables are shown here. Private methods go after public methods
 private:

  // Tokenize the input or not
  bool tokenizeinput_;

  // Add sentence markers (if missing) or not
  bool addsentencemarkers_;

  // Pointer to a wordmap object
  ucam::util::WordMapper *src2idxwmap_;

  // Key to find wordmap in the data object
  const std::string wordmapkey_;

  // Tokenization language. Only French is specified. There currently is a common scheme for english|spanish
  std::string tokenizelanguage_;

 public:

  /**
   * \brief Constructor
   * \param rg            pointer to RegistryPO object with all parsed parameters.
   * \param wordmapkey    registry key to access wordmap file name
   */
  PreProTask ( const ucam::util::RegistryPO& rg ,
               const std::string& wordmapkey = HifstConstants::kPreproWordmapLoad
             ) :
    wordmapkey_ ( wordmapkey ),
    addsentencemarkers_ ( rg.exists ( HifstConstants::kPreproAddsentencemarkers ) ),
    src2idxwmap_ ( NULL ),
    tokenizeinput_ ( rg.getBool ( HifstConstants::kPreproTokenizeEnable ) ),
    tokenizelanguage_ (rg.exists (HifstConstants::kPreproTokenizeLanguage)
                       ? rg.get<std::string> (HifstConstants::kPreproTokenizeLanguage) : "") {
  };

  ~PreProTask() {
  }

  inline void setTokenize ( bool tok ) {
    tokenizeinput_ = tok;
  };

  /**
   * \brief Reads an input sentence, tokenizes and integer-maps.
   */
  bool run ( Data& d ) {
    LINFO ( "Reading sentence #" << d.sidx );
    d.stats->setTimeStart ( "sent-dec" );
    if ( !USER_CHECK ( d.originalsentence != "",
                       "Empty source sentence?" ) ) return true;
    ucam::util::trim_spaces ( d.originalsentence, &d.originalsentence );
    if ( tokenizeinput_ ) {
      ucam::util::tokenize ( d.originalsentence, &d.tokenizedsentence ,
                             tokenizelanguage_ );
      LINFO ( "Tokenized :" << d.tokenizedsentence );
    } else d.tokenizedsentence = d.originalsentence;
    if ( addsentencemarkers_ )
      ucam::util::addSentenceMarkers ( d.tokenizedsentence );
    if ( d.wm.find ( wordmapkey_ ) != d.wm.end() ) src2idxwmap_ = d.wm[wordmapkey_];
    else src2idxwmap_ = NULL;
    if ( src2idxwmap_ ) {
      src2idxwmap_->reset_oov_id();
      ( *src2idxwmap_ ) ( d.tokenizedsentence, &d.sentence , true );
      d.oovwmap = src2idxwmap_->get_oovwmap();
      LINFO ( "mapped:" << d.sentence );
    } else d.sentence = d.tokenizedsentence;
    ucam::util::trim_spaces ( d.sentence, &d.sentence );
    if ( !USER_CHECK ( ucam::util::validate_source_sentence ( d.sentence ),
                       "Wrong sentence format, should be a sequence of numbers at this point!" ) ) {
      FORCELINFO ( "Bad Sentence:" << d.sentence );
      return true;
    }
    return false;
  };

 private:

  ZDISALLOW_COPY_AND_ASSIGN ( PreProTask );

};

}
}     // End namespaces

#endif
