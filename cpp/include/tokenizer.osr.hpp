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
/**
 * \file
 * \brief Lower casing/Tokenization/Detokenization not available for open source release
 *
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

namespace ucam {
namespace util {

///Not implemented, just pass through
inline void tokenize ( const std::string& is, std::string *os,
                       const std::string languagespecific = "" ) {
  *os = is;
};

///Not implemented, just pass through
inline void detokenize ( const std::string& is, std::string *os,
                         std::string languagespecific = "" ) {
  *os = is;
};

/**
 * \brief Adds sentence markers <s>, </s> to a sentence
 * \param sentence: sentence to modify (append,prepend) with the sentence markers.
 */
inline void addSentenceMarkers ( std::string& sentence ) {
  trim_spaces ( sentence, &sentence );
  if ( sentence.size() < 3 ) {
    sentence = "<s> " + sentence + " </s>";
    trim_spaces ( sentence, &sentence );
    return;
  } else if ( sentence.substr ( 0, 3 ) != "<s>" )
    sentence = "<s> " + sentence;
  if ( sentence.substr ( sentence.size() - 5, 4 ) != "</s>" )
    sentence += " </s>";
  trim_spaces ( sentence, &sentence );
};

/**
 * \brief Deletes sentence markers 1/2 or <s>/</s> for a sentence
 * \param sentence: sentence from which to delete markers
 */

inline void deleteSentenceMarkers ( std::string& sentence ) {
  boost::regex pattern1 ( "^\\s*1\\s|^\\s*<s>\\s+|^\\s*1\\s*$",
                          boost::regex_constants::icase | boost::regex_constants::perl );
  boost::regex pattern2 ( "\\s+2\\s*$|\\s+</s>\\s*$|^\\s*2\\s*$|^\\s*</s>\\s*$",
                          boost::regex_constants::icase | boost::regex_constants::perl );
  std::string replace ( "" );
  sentence = boost::regex_replace ( sentence, pattern1, replace );
  sentence = boost::regex_replace ( sentence, pattern2, replace );
  trim_spaces ( sentence, &sentence );
}

///Simple function that capitalizes first word and first word of sentence and first word
inline void capitalizeFirstWord ( std::vector<std::string>& words ) {
  // Always capitalize first word, or second if first is "...
  // Capitalize if previous is . or "
  USER_CHECK ( words.size(),
               "This function assumes non empty sequence of words!" );
  words[0][0] = toupper ( words[0][0] );
  for ( uint k = 1; k < words.size(); ++k ) {
    if ( words[k - 1] == "\"" || words[k - 1] == "." ) {
      words[k][0] = toupper ( words[k][0] );
    }
  }
};

///Alternative implementation using a string as input/output
inline void capitalizeFirstWord ( std::string& words ) {
  std::vector<std::string> w;
  boost::algorithm::split ( w, words, boost::algorithm::is_any_of ( "_" ) );
  capitalizeFirstWord ( w );
  words = boost::algorithm::join ( w, " " );
};

}
} // end namespaces

#endif
