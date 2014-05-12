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

#ifndef DATA_CYKPARSER_CYKBACKPOINTERS_HPP
#define DATA_CYKPARSER_CYKBACKPOINTERS_HPP

/**
 * \file
 * \brief Contains functor that provides access to cyk backpointers
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {
/**
 * \brief functor that provides cyk backpointers
 *
 */
class CYKbackpointers {
  typedef unordered_map<unsigned, cykparser_ruledependencies_t  >
  cykparser_backpointers_t;

 private:
  ///Backpointer structure
  cykparser_backpointers_t bp_;
 public:

  //Get the set of backpointers for given coordinates (cc,x,y)
  inline const cykparser_ruledependencies_t& operator() ( const unsigned cc,
      const unsigned x, const unsigned y ) {
    return bp_[APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG];
  }

  ///Add all the set of backpointers to the grid
  inline void Add ( const unsigned cc, const unsigned x, const unsigned y,
                    const cykparser_ruledependencies_t& coords ) {
    if ( coords.size() )
      bp_[APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG] = coords;
  };

  ///Add set of backpointers to the grid
  inline void Add ( const unsigned cc, const unsigned x, const unsigned y,
                    const cykparser_rulebpcoordinates_t& coords ) {
    if ( coords.size() )
      bp_[APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG].push_back ( coords );
  };

  ///Size of the backpointer structure (number of cc,x,y elements inserted).
  inline std::size_t size() {
    return bp_.size();
  };

  ///Delete cyk backpointers
  inline void reset() {
    bp_.clear();
  };

};

}
}  // end namespaces

#endif
