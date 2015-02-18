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

#ifndef TASK_ALILATS2SPLATS_HPP
#define TASK_ALILATS2SPLATS_HPP

/** \file
 *\brief Data object for alilats to sparse weight lats binary
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 *\brief Data class containing relevant variables. To be used as template for task classes using it.
 *
 */
template <class ArcT = void >
class AlilatsToSparseWeightLatsData {
 public:
  AlilatsToSparseWeightLatsData() :
    sidx ( 0 ),
    stats ( new ucam::fsttools::StatsData ) {
  };

  /// Sentence index
  uint sidx;

  ///Pointers to lattices (e.g. translation lattice, lmbr, etc) , accessed by unique keys. Notice that it is a void pointer, so any type of Fst/Arc will fit in nicely.
  unordered_map<std::string, void * > fsts;

  ///Collections of language models accessed by keys
  unordered_map<std::string, std::vector <const ucam::fsttools::KenLMData *> >
  klm;

  boost::scoped_ptr<ucam::fsttools::StatsData>  stats;

  ///Wordmap/Integer map objects. Will be necessary if alilats2splats uses non integer-mapped language models
  unordered_map<std::string, ucam::util::WordMapper *> wm;

};

}
} // end namespaces

#endif //TASKALILATS2SPLATS_HPP

