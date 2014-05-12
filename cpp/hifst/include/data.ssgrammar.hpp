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

#ifndef SENTENCESPECIFICGRAMMARDATA_HPP
#define SENTENCESPECIFICGRAMMARDATA_HPP

/**
 * \file
 * \brief Contains sentence-specific grammar data.
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 * \remark This file has been reviewed/modified by:
 */

namespace ucam {
namespace hifst {

/**
 *\brief Structure for sentence-specific grammar
 * Rules will be queried by cyk per position and number of elements in the right-hand-side (source) of the rule
 * Therefore indices are stored in this fashion so queries can be done directly.
 * Note: a more efficient implementation could be to store these rule indices in a structure much closer even to the cyk grid.
 * For instance, if an instanced pattern has never been seen below span 5, there is no need for the cyk to query and
 * reject it.
 */

struct SentenceSpecificGrammarData {

  SentenceSpecificGrammarData() :
    grammar ( NULL ) {
#ifdef USE_GOOGLE_SPARSE_HASH
    extrarules.set_empty_key ( std::numeric_limits<std::size_t>::max() );
#endif
  }

  ///Pointer to the original grammar data (no ownership)
  const GrammarData *grammar;

  ///Sentence-specific data.
  ///cells containing potentially applicable rules with one element
  ssgrammar_rulesmap_t rulesWithRhsSpan1;
  ///cells containing potentially applicable rules with two or more elements
  ssgrammar_rulesmap_t  rulesWithRhsSpan2OrMore;

  ///Extra rules, e.g. due to oovs, feedback, etc. Indexed by a rule id that must be bigger than the number of rules of the whole grammar
  ///Note that this hash is not expected to contain many rules. If so, access speed will degrade.
#ifndef USE_GOOGLE_SPARSE_HASH
  unordered_map<std::size_t, std::string> extrarules;
#else
  google::dense_hash_map<std::size_t, std::string> extrarules;
#endif
  ///\todo  All these methods could be class const if we used .at(idx) -> c++11. Check them carefully

  inline void reset() {
    rulesWithRhsSpan1.clear();
    rulesWithRhsSpan2OrMore.clear();
    extrarules.clear();
    grammar = NULL;
  }

  ///Returns rule corresponding to index idx
  inline const std::string getRule ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRule ( idx );
    LINFO ( "ssgrammar idx=" << idx );
    return extrarules[idx];
  };

  ///Returns Left-hand-side of a rule corresponding to index idx
  inline const std::string getLHS ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getLHS ( idx );
    std::size_t pos = extrarules[idx].find_first_of ( " " );
    return extrarules[idx].substr ( 0, pos );
  };

  ///Returns Right-hand-side (source) of the rule with index=idx
  inline const std::string getRHSSource ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSSource ( idx );
    std::size_t pos = extrarules[idx].find_first_of ( " " ) + 1;
    std::size_t pos2 = extrarules[idx].find_first_of ( " ", pos );
    return extrarules[idx].substr ( pos, pos2 - pos );
  };

  ///Returns element at position rulepos of right-hand-side (source)
  inline const std::string getRHSSource ( std::size_t idx, uint rulepos ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSSource ( idx, rulepos );
    std::size_t pos = extrarules[idx].find_first_of ( " " );
    std::size_t j = pos , jold;
    for ( uint k = 0; k <= rulepos; ++k ) {
      jold = j;
      j = extrarules[idx].find_first_of ( "_ ", jold + 1 );
      if ( j == std::string::npos )
        if ( rulepos ) return "";
    }
    return extrarules[idx].substr ( jold + 1, j - jold - 1 );
  };

  ///Returns vector of elements of the RHS source
  inline const std::vector<std::string> getRHSSplitSource ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSSplitSource ( idx );
    std::vector<std::string> splitsource;
    boost::algorithm::split ( splitsource, getRHSSource ( idx ),
                              boost::algorithm::is_any_of ( "_" ) );
    return splitsource;
  };

  ///Returns size of RHS source of a rule
  inline const uint getRHSSourceSize ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSSourceSize ( idx );
    std::size_t pos  = extrarules[idx].find_first_of ( " " ) + 1;
    std::size_t pos1 = extrarules[idx].find_first_of ( " " , pos ) + 1;
    return ucam::util::count_needles ( extrarules[idx], '_', pos, pos1 ) + 1 ;
  };

  /// Returns RHS translation of a rule with index idx
  inline const std::string getRHSTranslation ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSTranslation ( idx );
    std::size_t pos  = extrarules[idx].find_first_of ( " " ) + 1;
    std::size_t pos1 = extrarules[idx].find_first_of ( " ", pos ) + 1;
    std::size_t pos2 = extrarules[idx].find_first_of ( " ", pos1 );
    return extrarules[idx].substr ( pos1, pos2 - pos1 );
  };

  ///Returns translation as a vector of elements
  inline const std::vector<std::string> getRHSSplitTranslation (
    std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSSplitTranslation ( idx );
    std::vector<std::string> splittranslation;
    boost::algorithm::split ( splittranslation, getRHSTranslation ( idx ),
                              boost::algorithm::is_any_of ( "_" ) );
    return splittranslation;
  };

  /// Returns size of RHS (translation) of a rule
  inline const uint getRHSTranslationSize ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getRHSTranslationSize ( idx );
    std::size_t pos  = extrarules[idx].find_first_of ( " " ) + 1;
    std::size_t pos1 = extrarules[idx].find_first_of ( " ", pos ) + 1;
    std::size_t pos2 = extrarules[idx].find_first_of ( " ", pos1 );
    return ucam::util::count_needles ( extrarules[idx], '_', pos1, pos2 ) + 1;
  };

  ///Returns the weight of a rule. This weight is the dot product of all the features with its scales.
  inline const float getWeight ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getWeight ( idx );
    float weight;
    std::size_t pos  = extrarules[idx].find_first_of ( " " ) + 1;
    std::size_t pos1 = extrarules[idx].find_first_of ( " ", pos );
    std::size_t pos2 = extrarules[idx].find_first_of ( " ", pos1 + 1 );
    std::size_t pos3 = extrarules[idx].find_first_of ( " \n\0", pos2 + 1 );
    return ucam::util::toNumber<float> ( extrarules[idx].substr ( pos2,
                                         pos3 - pos2 ) );
  };
  inline const bool isPhrase ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->isPhrase ( idx );
    std::size_t pos0  = extrarules[idx].find_first_of ( " " ) + 1;
    std::size_t pos = extrarules[idx].find_first_of ( " ", pos0 );
    for ( const char *c = extrarules[idx].c_str() + pos0;
          c <= extrarules[idx].c_str() + pos; ++c )
      if ( *c >= 'A' && *c <= 'Z' ) return false; //has non-terminals.
    return true; //pure phrase.
  };
  ///Returns the true idx of a rule (i.e. line in the grammar file). If it is sentence specific, then return the idx itself.
  inline const std::size_t getIdx ( std::size_t idx ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->getIdx ( idx );
    return idx;
  };

  ///Determines whether a rule is allowed within the vocabulary. If the rule is sentence-specific (i.e. created due to oov or feedback,etc)
  ///this method will always return true.
  inline const bool isAcceptedByVocabulary ( const std::size_t idx,
      const unordered_set<std::string>& vcb ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      return grammar->isAcceptedByVocabulary ( idx, vcb );
    return true;
  };

  /**
   * \brief  Returns the non-terminal mappings for a rule. For more details see getRuleMappings function.
   * \param idx                rule (sorted) identifier.
   * \param mappings           Source to target non-terminal mapping information
   */
  inline void getMappings ( std::size_t idx,
                            unordered_map<uint, uint> *mappings ) {
    if ( extrarules.find ( idx ) == extrarules.end() )
      grammar->getMappings ( idx, mappings );
    if ( isPhrase ( idx ) ) return;
    std::vector<std::string> source = getRHSSplitSource ( idx );
    std::vector<std::string> translation = getRHSSplitTranslation ( idx );
    getRuleMappings ( source, translation, mappings );
  };
};

/// Convenience function to detect whether a phrase is actually a single word.
/// IMPORTANT: By convention, non-terminals have a first capital letter followed by any number of letters/numbers.
/// If the last character is a number, it will be considered as an index, not therefore in the definition of the non-terminal, e.g. X,X1,X2 are all X.
inline bool phraseIsTerminalWord ( const std::string& phrase ) {
  for ( uint k = 0; k < phrase.size(); ++k ) {
    if ( phrase[k] >= 'A' && phrase[k] <= 'Z' ) return false;
    else if ( phrase[k] == '_' ) return false;
  }
  return true;
};

}
}  // end namespaces

#endif
