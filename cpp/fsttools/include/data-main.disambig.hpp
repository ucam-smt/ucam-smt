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
 * \brief Data object for disambig tool
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {
/**
 * \brief data structure for disambig tool. It needs at least a mixed case language model, mixed-case vocabulary and a transduction unigram model.
 *
 */

struct DisambigData {
  DisambigData() :
    sidx ( 0 ),
    recasingvcblm ( NULL ),
    stats ( new StatsData ) {
  };

  ///Sentence index
  uint sidx;

  ///lists of language models indexed by a key (i.e. parameter )
  unordered_map<std::string, std::vector <const KenLMData*> > klm;
  ///Mixed case vocabulary used by the unigram model and the language model
  unordered_set<std::string> *recasingvcblm;
  ///Unigram model to be stored here
  unordered_map<std::string, void * > fsts;

  StatsData  *stats;

  ///Wordmap/Integer map objects
  unordered_map<std::string, ucam::util::WordMapper *> wm;

};

}
} // end namespaces
#endif
