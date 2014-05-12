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

#ifndef CYKDATA_HPP
#define CYKDATA_HPP

/**
 * \file
 * \brief Contains structures and classes for GrammarData.
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 */

#include "data.cykparser.cykgrid.hpp"
#include "data.cykparser.cykbackpointers.hpp"

namespace ucam {
namespace hifst {
/**
 * \brief Data structure containing all cyk-related information
 *
 */
struct CYKdata {

  ///The sentence we want to parse.
  cykparser_sentence_t sentence;

  /// Map between categories (S=1,X=2,...)
  grammar_categories_t categories;

  /// Inverse map (1=S,2=X,...)
  grammar_inversecategories_t vcat;

  ///Cyk grid. Each cell of the grid is uniquely defined by three dimensions: [category,x,y].
  CYKgrid cykgrid;

  ///Backpointers for each rule stored in the cyk grid to lower level cells.
  ///Each backpointer must identify the three dimensions of the cell.
  ///Each rule has as many backpointers as non-terminals.
  CYKbackpointers bp;

  /// Success and how many parse S nodes have been found in the topmost cell. If 0, cyk parser has failed.
  uint success;

  ///coordinate dependencies for each candidate.
  cykparser_ruledependencies_t rd;

  ///number of non-terminals
  uint nnt;

  // Non-terminals are bounded by a maximum span threshold (i.e. typically --hifst.hrmaxspan=10).
  // However, certain non-terminals in a hiero-style grammar
  // must be able to reach higher spans. Most notably, this is the case of non-terminal S, used as glue rule.
  // The list of exceptions are included in nt_exceptions_maxspan.
  unordered_set<std::string> nt_exceptions_maxspan;

  /**
   * \remarks     Frees memory.
   */
  int freeMemory() {
    for ( unsigned int k = 0; k < rd.size(); k++ ) {
      rd[k].clear();
    }
    rd.clear();
    cykgrid.reset();
    bp.reset();
    categories.clear();
    vcat.clear();
    sentence.clear();
    success = 0;
    nnt = 0;
    return 0;
  };

  /**
   * \brief Returns number of words in the sentence.
   */

  inline uint getNumberWordsSentence() {
    return sentence.size();
  };

  /**
   * \brief Stores rules in a simple hash.
   * \param c: hash to fill. Keys based on cell grid axes. Values are rule (sorted) ids.
   */
  void storeRules ( unordered_map<uint, std::vector<uint> >& c ) {
    for ( unsigned int cc = 1; cc <= nnt; cc++ ) {
      for ( unsigned int x = 0; x < sentence.size(); x++ ) {
        for ( unsigned int y = 0; y < sentence.size() - x; y++ ) {
          for ( unsigned int k = 0; k < cykgrid ( cc, x, y ).size(); ++k )
            c[cc * 1000000 + y * 1000 + x].push_back ( cykgrid ( cc, x, y, k ) );
        }
      }
    }
  };

  /**
   * \brief Stores rule counts in a simple hash.
   * \param c: hash to fill. Keys based on cell grid axes. Values are rule counts per cell.
   */

  void storeRuleCounts ( unordered_map<uint, uint>& c ) {
    for ( unsigned int cc = 1; cc <= nnt; cc++ ) {
      for ( unsigned int x = 0; x < sentence.size(); x++ ) {
        for ( unsigned int y = 0; y < sentence.size() - x; y++ ) {
          if ( cykgrid ( cc, x, y ).size() )
            c[cc * 1000000 + y * 1000 + x] = cykgrid ( cc, x, y ).size();
        }
      }
    }
  };

};

}
} // end namespaces

#endif
