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

#ifndef DATA_MAIN_APPLYLM_HPP
#define DATA_MAIN_APPLYLM_HPP

/**
 * \file
 * \brief Data object for applylm tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {
/**
 * \brief data structure for applylm tool
 *
 */
template <class ArcT = void  >
struct ApplyLMData {
  ApplyLMData() :
    sidx ( 0 ),
    stats ( new StatsData ) {
  };
  uint sidx;

  ///lists of language models indexed by a key (i.e. parameter )
  unordered_map<std::string, std::vector <const KenLMData *> > klm;

  StatsData  *stats;

  unordered_map<std::string, void *> fsts;

  ///Wordmap/Integer map objects
  unordered_map<std::string, ucam::util::WordMapper *> wm;
};

}
} // end namespaces

#endif
