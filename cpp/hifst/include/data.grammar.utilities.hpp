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

#ifndef DATA_GRAMMAR_UTILITIES_HPP
#define DATA_GRAMMAR_UTILITIES_HPP

/**
 * \file
 * \brief Contains structures and classes for GrammarData
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief A generic element counter that can be used to any string.
 * It is intended to use with either source or target side of a rule (right-hand side parts of the synchronous rule).
 * \param rhs: source or target.
 */
inline const uint getSize ( const std::string& rhs ) {
  if ( rhs == "" ) return 0;
  return ucam::util::count_needles ( rhs, '_', 0, rhs.size() ) + 1;
}

/**
 * \brief Determine if the element is a terminal (i.e. a word, represented by a number)
 * or a non-terminal (i.e. ^[A-Z]+(0-9)?). Only first position is checked.
 * \param word: the element we are checking.
 */

inline bool isTerminal ( const std::string& word ) {
  if ( word[0] < 'A' || word[0] > 'Z' ) return true;
  return false;
};

/**
 * \brief Return the filtered non-terminal name.
 * For example, for the rule Z 3_XT2_5 XT2,
 * getFilteredNonTerminal("XT2") should return XT.
 * \param word: The non-terminal we want to filter.
 */

inline void getFilteredNonTerminal ( std::string& word ) {
  if ( isTerminal ( word ) ) return;
  if ( word[word.size() - 1] >= '0' && word[word.size() - 1] <= '9' )
    word.resize ( word.size() - 1 );
};

/**
 * \brief Given a source and translation of the same rule, sharing the same non-terminals in RHS,
 * returns correspondences between source and target non-terminal indices.
 * For example:
 * X a_Y_Z_b c_Z_Y_d: mappings[0]=1; mappings[1]=0.
 *
 * \param source          RHS source of a rule
 * \param translation     RHS target of a rule
 * \param mappings        After finished, contains correspondences between source and target non-terminal indexes
 */

inline void getRuleMappings ( const std::vector<std::string>& source,
                              const std::vector<std::string>& translation ,
                              unordered_map<uint, uint> *mappings ) {
  unordered_map<std::string, uint> partial_mappings;
  uint nt = 0;
  for ( uint k = 0; k < source.size(); ++k ) {
    if ( isTerminal ( source[k] ) ) continue;
    partial_mappings[source[k]] = nt++;
  }
  nt = 0;
  for ( uint k = 0; k < translation.size(); ++k ) {
    if ( isTerminal ( translation[k] ) ) continue;
    USER_CHECK ( partial_mappings.find ( translation[k] ) != partial_mappings.end(),
                 "RHS source and RHS target do not match!" );
    ( *mappings ) [partial_mappings[translation[k]]] = nt++;
  }
  return;
};

}
} // end namespaces

#endif
