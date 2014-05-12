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

#ifndef TASK_HIFST_LOCALPRUNINGCONDITIONS_HPP
#define TASK_HIFST_LOCALPRUNINGCONDITIONS_HPP

/**
 * \file
 * \brief Contains functor and struct  to handle local pruning conditions
 * \date 20-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

///struct containing the elements that trigger local pruning.
struct conditions {
  conditions() :
    cat ( 0 ),
    span ( 0 ),
    lpns ( 0 ),
    lpw ( 0.0f ) {
  };

  conditions ( unsigned category, unsigned span, unsigned numstates,
               float weight ) :
    cat ( category ),
    span ( span ),
    lpns ( numstates ),
    lpw ( weight ) {
  };
  ///Category
  unsigned cat;
  ///span
  unsigned span;
  ///number of states
  unsigned lpns;
  ///weight
  float lpw;
} ;

/**
 * \brief convenience class that takes care of local pruning conditions.
 * Conditions are indexed by 1000*cc+y, so you can search through all conditions
 * and get to the closest set of conditions that apply.
 */

class LocalPruningConditions {
  //Private variables are shown here. Private methods go after public methods
 private:

  ///maps 1000*cc+y to local conditions. Must be map to use lower_bound.
  std::map<uint64, conditions> lpc_;
 public:
  ///Empty constructor
  LocalPruningConditions() {};

  ///Add condition.
  inline void add ( const conditions& c ) {
    lpc_[c.cat * 100000000000 + c.span * 100000000 + c.lpns % 100000000 ] = c;
  };

  /**
   * \brief Checks whether a given cell lattice at (cc,x,y) with numstates states qualifies for local pruning.
   * \param cc                Category numerical representation for the cyk cell
   * \param span              Corresponding to y coordinate of cyk cell. Note that span=y+1
   * \param numstates         Number of states of the cell lattice (1 - 100M states)
   * \param w                 If qualifies, weight for pruning will be stored here
   * \returns true if qualifies, false otherwise
   */
  bool operator () ( unsigned cc, unsigned span, unsigned numstates , float& w ) {
    if ( ! lpc_.size() ) return false;
    uint64 catspan = cc * 100000000000 + span * 100000000 +
                     ( numstates % 100000000 );
    ///the underlying assumption is that no sentence will have more than 999 words
    std::map <uint64, conditions>::iterator itx;
    itx = lpc_.lower_bound ( catspan );
    if ( itx == lpc_.end() ) --itx;
    else if ( itx->first != catspan && itx != lpc_.begin() ) --itx;
    while  ( itx->second.cat == cc && ( itx->second.span > span
                                        || itx->second.lpns > numstates ) && itx != lpc_.begin() ) --itx;
    LINFO ( "AT " << cc << ", ? ," << span - 1  << ": actual cell=" << catspan <<
            " closest conditions=" << itx->first );
    if ( itx->second.cat != cc || itx->second.span > span
         || itx->second.lpns > numstates ) return false;
    w = itx->second.lpw;
    return true;
  };

  ///returns size of map
  inline std::size_t size() {
    return lpc_.size();
  };

  ///Clears all conditions
  inline void clear() {
    lpc_.clear();
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( LocalPruningConditions );

};

}
} // end namespaces

#endif //TASK_HIFST_LOCALPRUNINGCONDITIONS_HPP
