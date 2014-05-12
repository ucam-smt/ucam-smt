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

#ifndef TASK_HIFST_EXPANDEDNUMSTATES_HPP
#define TASK_HIFST_EXPANDEDNUMSTATES_HPP

/**
 * \file
 * \brief Contains utility class to predict number of states of an RTN
 * after expanding to equivalent FSA
 * \date 22-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Utility class that, given an RTN with root at (cc,x,y), estimates the number of states of an expanded FSA
 * The RTN is not explicitly passed to this class. Instead, it is updated sequentially as individual FSAs
 * are created.
 */

template<class Arc>
class ExpandedNumStatesRTN {
 private:
  ///Expanded number of states for FSAs at cc,x,y if they were root
  unordered_map< uint , uint > rtnnumstates_;
 public:
  ///Constructor
  ExpandedNumStatesRTN() {};

  ///Returns estimated number of states for an rtn with a root fst at (cc,x,y)
  inline uint operator() ( uint cc, uint x, uint y ) {
    uint hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    return rtnnumstates_[hieroindex];
  };

  /**
   *\brief Estimates (expanded) number of states of rtn_[cc][x][y]
   * Traverses arcs and and add number of states of other lower-level rtns.
   * Assuming minimized FSA, substract number 2 per lowel level rtn: 1 for final state and another for for initial state.
   * This should provide a good estimate of the size of the expanded fst (equivalent roughly to Replace+RmEpsilon).
   * \param cc       Non-terminal id (starting at 1)
   * \param x        Position in sentence (starting at 0)
   * \param y        Span - 1.
   * \param myfst    Pointer to an fst corresponding to cell cc,x,y
   */

  void update ( uint cc, uint x, uint y, fst::VectorFst<Arc> *myfst ) {
    uint hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    uint& ns = rtnnumstates_[hieroindex];
    ns = myfst->NumStates();
    LDEBUG ( "AT: " << cc << "," << x << "," << y << ":" << ",NS=" <<
             rtnnumstates_[hieroindex] );
    typedef typename Arc::StateId StateId;
    for ( fst::StateIterator< fst::VectorFst<Arc> > si ( *myfst ); !si.Done();
          si.Next() ) {
      StateId state_id = si.Value();
      for ( fst::ArcIterator< fst::VectorFst<Arc> > ai ( *myfst , si.Value() );
            !ai.Done(); ai.Next() ) {
        const Arc& arc = ai.Value();
        if ( arc.ilabel < APBASETAG ) continue;
        ns += rtnnumstates_[arc.ilabel] - 2;
      }
    }
    LINFO ( "AT: " << cc << "," << x << "," << y << ":" << "NS (expanded) =" <<
            ns );
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( ExpandedNumStatesRTN );
};

}
} // end namespaces

#endif //ifndef TASK_HIFST_EXPANDEDNUMSTATES_HPP
