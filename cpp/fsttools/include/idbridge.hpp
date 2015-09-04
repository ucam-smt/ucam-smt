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

#ifndef IDBRIDGE_HPP
#define IDBRIDGE_HPP

/**
 * \file
 * \brief maps between grammar targets ids and lm ids
 * \remarks For historical and philosophical reasons
 * we want to be able to integer-map our data
 * externally and we don't want to depend on kenlm
 * internal integer mapping.
 * IdBridge maps between grammar and lm ids.
 */

namespace ucam {
namespace fsttools {

class IdBridge {
 private:
  typedef std::tr1::unordered_map<unsigned,unsigned> MapType;
  // rmapper (reverse) for debugging purposes only
  MapType mapper, rmapper;
  // output mapper for nplm with two vocabularies
  MapType omapper, romapper;
 public:
  IdBridge() {}

  inline unsigned const map (unsigned idx) const {
    MapType::const_iterator itx = mapper.find (idx);
    if (itx != mapper.end() ) {
      return itx->second;
    }
    return 0;
  };

  inline unsigned const rmap (unsigned idx) const {
    MapType::const_iterator itx = rmapper.find (idx);
    if (itx != rmapper.end() )
      return itx->second;
    return 0;
  };


  inline unsigned const mapOutput (unsigned idx) const {
    MapType::const_iterator itx = omapper.find (idx);
    if (itx != omapper.end() ) {
      return itx->second;
    }
    return 0;
  };

  inline unsigned const rmapOutput (unsigned idx) const {
    MapType::const_iterator itx = romapper.find (idx);
    if (itx != romapper.end() )
      return itx->second;
    return 0;
  };


  inline void add (unsigned grammar_idx, unsigned lm_idx) {
    LDEBUG ("grammar idx=" << grammar_idx << ", lm_idx=" << lm_idx);
    mapper[grammar_idx] = lm_idx;
#ifdef PRINTDEBUG1
    rmapper[lm_idx] = grammar_idx;
#endif
  }


  inline void addOutput (unsigned grammar_idx, unsigned lm_idx) {
    LDEBUG ("ovocab: grammar idx=" << grammar_idx << ", lm_idx=" << lm_idx);
    omapper[grammar_idx] = lm_idx;
#ifdef PRINTDEBUG1
    romapper[lm_idx] = grammar_idx;
#endif
  }
};

}} // end namespaces
#endif
