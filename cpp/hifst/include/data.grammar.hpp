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

#ifndef DATA_GRAMMAR_HPP
#define DATA_GRAMMAR_HPP

/**
 * \file
 * \brief Contains structures and classes for GrammarData
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include "data.grammar.utilities.hpp"
#include "data.grammar.comparetool.hpp"

namespace ucam {
namespace hifst {
/**
 *\brief Struct containing grammar rules.
 *
 * Contains the grammar in a string, along with a set of sorted indices telling where each rule can be found in the string.
 * This struct is typically generated by a GrammarTask and used by several other tasks.
 *
 * Patterns, if precalculated, are also available in this struct.
 * Indices have been sorted according to a comparison object. This object is required for further access and is made available through a pointer.
 * For instance, in a synchronous grammar we need to sort the rules according to an abstraction (i.e. all non-terminals are represented by the same
 * capital letter).
 */

struct GrammarData {

  ///GrammarData constructor. Initializes GrammarData with empty information.
  GrammarData() :
    vpos ( NULL ),
    sizeofvpos ( 0 ),
    ct ( NULL ) {
  };

  ///Destructor
  ~GrammarData() {
    if ( vpos != NULL ) delete [] vpos;
  }

  /// The whole grammar.
  std::string filecontents;
  /// Sorted Indices.
  posindex *vpos;
  /// Number of rules
  std::size_t sizeofvpos;
  /// Patterns in these rules
  std::unordered_set<std::string> patterns;
  /// Pointer to a Comparison object, assumed no ownership
  CompareTool *ct;

  ///Ordered list of non-terminals (listed in hierarchical order according to identity rules)
  grammar_categories_t categories;
  grammar_inversecategories_t vcat;

  ///Reset object
  inline void reset() {
    filecontents = "";
    if ( vpos != NULL ) delete [] vpos;
    patterns.clear();
    categories.clear();
    vcat.clear();
    sizeofvpos = 0;
    ct = NULL;
  }

  ///Gets a rule indexed by idx. Rule format: LHS RHSSource RHSTarget weight
  inline const std::string getRule ( std::size_t idx ) const {
    std::size_t rpos = vpos[idx].p - vpos[idx].o;
    std::size_t pos = filecontents.find_first_of ( "\n", rpos );
    return filecontents.substr ( rpos, pos - rpos );
  }

  ///Gets left-hand-side of the rule indexed by idx
  inline const std::string getLHS ( std::size_t idx ) const {
    std::size_t rpos = vpos[idx].p - vpos[idx].o;
    return filecontents.substr ( rpos, vpos[idx].p - rpos - 1 );
  }

  ///Gets right-hand-side source for a rule using rule index idx
  inline const std::string getRHSSource ( std::size_t idx ) const {
    std::size_t pos = filecontents.find_first_of ( " ", vpos[idx].p );
    return filecontents.substr ( vpos[idx].p, pos - vpos[idx].p );
  }

  ///Gets element at position rulepos from the right-hand-side source for a rule indexed by idx.
  inline const std::string getRHSSource ( std::size_t idx, uint rulepos ) const {
    std::size_t pos = filecontents.find_first_of ( " ", vpos[idx].p );
    std::size_t j = vpos[idx].p - 1, jold;
    for ( uint k = 0; k <= rulepos; ++k ) {
      jold = j;
      j = filecontents.find_first_of ( "_ ", jold + 1 );
      if ( j == std::string::npos )
        if ( rulepos ) return "";
    }
    return filecontents.substr ( jold + 1, j - jold - 1 );
  }

  ///Gets a splitted version of RHS (source)
  inline const std::vector<std::string> getRHSSplitSource (std::size_t idx ) const {
    std::vector<std::string> splitsource;
    boost::algorithm::split ( splitsource, getRHSSource ( idx )
                              , boost::algorithm::is_any_of ( "_" ) );
    return splitsource;
  }

  ///Gets number of elements in the RHS source
  inline const uint getRHSSourceSize ( std::size_t idx ) const {
    std::size_t pos = filecontents.find_first_of ( " ", vpos[idx].p );
    return ucam::util::count_needles ( filecontents, '_', vpos[idx].p, pos ) + 1;
  }

  ///Returns RHS translation part of a rule accessed by index idx
  inline const std::string getRHSTranslation ( std::size_t idx ) const {
    std::size_t pos = filecontents.find_first_of ( " ", vpos[idx].p ) + 1;
    std::size_t pos2 = filecontents.find_first_of ( " ", pos );
    return filecontents.substr ( pos, pos2 - pos );
  }

  ///Returns the translation as a vector of elements
  inline const std::vector<std::string> getRHSSplitTranslation (
    std::size_t idx ) const {
    std::vector<std::string> splittranslation;
    boost::algorithm::split ( splittranslation, getRHSTranslation ( idx ),
                              boost::algorithm::is_any_of ( "_" ) );
    return splittranslation;
  }

  ///Returns the number of elements in translation for a given rule
  inline const uint getRHSTranslationSize ( std::size_t idx ) const {
    std::size_t pos = filecontents.find_first_of ( " ", vpos[idx].p ) + 1;
    std::size_t pos2 = filecontents.find_first_of ( " ", pos );
    return ucam::util::count_needles ( filecontents, '_', pos, pos2 ) + 1;
  }

  ///Returns weight of a rule accessed by index idx
  inline const float getWeight ( std::size_t idx ) const {
    std::size_t pos1 = filecontents.find_first_of ( " ", vpos[idx].p );
    std::size_t pos2 = filecontents.find_first_of ( " ", pos1 + 1 );
    std::size_t pos3 = filecontents.find_first_of ( " \t\n\0", pos2 + 1 );
    return ucam::util::toNumber<float> ( filecontents.substr ( pos2,
                                         pos3 - pos2 ) );
  }

  // Affiliation or alignments go physically after the weight, so that
  // it is an optional field.
  void getLinks(std::size_t idx
                , std::vector<unsigned> &links ) const {
    using namespace std;
    using namespace boost::algorithm;
    size_t pos1 = filecontents.find_first_of ( " ", vpos[idx].p );
    size_t pos2 = filecontents.find_first_of ( " ", pos1 + 1 );
    size_t pos3 = filecontents.find_first_of ( "\t\n\0", pos2 + 1 );
    if (filecontents[pos3] == '\t') {
      size_t pos4 = filecontents.find_first_of ( " \t\n\0", pos3 + 1 );
      string y = filecontents.substr ( pos3 + 1, pos4 - pos3 - 1);
      LDEBUG("Links=[" << y << "]");
      vector<string> x;
      split(x, y, is_any_of("_"));
      if (links.size() != x.size()) {
        LERROR("Houston! " << idx << "=>" << y << ",x.size=" << x.size() << ",links.size=" << links.size() );
        exit(EXIT_FAILURE);
      }
      for (unsigned k = 0; k < x.size(); ++k) {
        LDEBUG("x at " << k << "=" << x[k] << ";");
        ucam::util::toNumber<unsigned>("0");
        ucam::util::toNumber<unsigned>("1");
        links[k] = ucam::util::toNumber<unsigned>(x[k]);
      }
    }
  }

  ///Checks whether the rule is a phrase or not (i.e. is hierarchical)
  inline const bool isPhrase ( std::size_t idx ) const {
    std::size_t pos = filecontents.find_first_of ( " ", vpos[idx].p );
    for ( const char *c = filecontents.c_str() + vpos[idx].p;
          c <= filecontents.c_str() + pos; ++c )
      if ( *c >= 'A' && *c <= 'Z' ) return false; //has non-terminals.
    return true; //pure phrase.
  }

  ///Gets the real position (line) in the (potentially unsorted) file.
  inline const std::size_t getIdx ( std::size_t idx ) const {
    return vpos[idx].order;
  }

  /**
   * \brief  Returns the non-terminal mappings. For more details see getRuleMappings function
   * \param idx           rule (sorted) identifier.
   * \param mappings      On completion, non-terminal mappings from source to target will be stored here.
   */
  void getMappings ( std::size_t idx,
                     unordered_map<uint, uint> *mappings ) const {
    if ( isPhrase ( idx ) ) return;
    const std::vector<std::string> source = getRHSSplitSource ( idx );
    const std::vector<std::string> translation = getRHSSplitTranslation ( idx );
    getRuleMappings ( source, translation, mappings );
    return;
  }

  /**
   * \brief Determines whether a particular rule is allowed within a vocabulary,
   * i.e. all target words of the rule exist within this vocabulary.
   * NOTE: If vocabulary variable is empty, it will always return true,
   * this is, no vocabulary restriction is applied.
   * \param idx : rule index
   * \param vcb : vocabulary to check against
   */

  inline const bool isAcceptedByVocabulary ( const std::size_t idx,
      const std::unordered_set<std::string>& vcb ) const {
    if ( !vcb.size() ) return true;
    std::vector<std::string> tx = getRHSSplitTranslation ( idx );
    for ( uint k = 0; k < tx.size(); ++k ) {
      if ( tx[k] == "<dr>" || tx[k] == "<oov>" || tx[k] == "<s>" || tx[k] == "</s>"
           || tx[k] == "<sep>") continue;
      if ( !isTerminal ( tx[k] ) ) continue;
      if ( vcb.find ( tx[k] ) == vcb.end() ) return false;
    }
    return true;
  };

};

}
} // end namespaces

#endif
