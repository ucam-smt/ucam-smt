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

#ifndef DATA_CYKPARSER_CYKGRID_HPP
#define DATA_CYKPARSER_CYKGRID_HPP

/**
 * \file
 * \brief Contains functor for the cyk grid.
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief functor that provides cykgrid access methods
 *
 */
class CYKgrid {
  typedef unordered_map< uint, ssgrammar_listofrules_t  >  cykparser_cykgrid_t;
 private:

  cykparser_cykgrid_t cykgrid_;
 public:

  ///Get list of grammar rules assigned to (cc,x,y)
  inline const ssgrammar_listofrules_t& operator() ( const uint cc, const uint x,
      const uint y ) {
    return cykgrid_[APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG];
  };

  ///Get a specific rule at (cc,x,y) corresponding to index rulepos
  inline uint operator() ( const uint cc, const uint x, const uint y,
                           const uint rulepos ) {
    return cykgrid_[ APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG ][rulepos];
  };

  ///Add a rule to the cyk grid at (cc,x,y)
  inline void Add ( const uint cc, const uint x, const uint y,
                    const uint ruleidx ) {
    cykgrid_[ APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG ].push_back (
      ruleidx );
  };

  ///Clear cyk grid
  inline void reset() {
    cykgrid_.clear();
  };
  ///Return actual size of the cyk grid
  inline std::size_t size() {
    return cykgrid_.size();
  };

};

}
} // end namespaces

#endif //DATA_CYKPARSER_CYKGRID_HPP
