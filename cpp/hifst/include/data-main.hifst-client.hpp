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

#ifndef TASKDATA_HPP
#define TASKDATA_HPP

/** \file
 *\brief Data object for hifst-client
 * \date October 2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 *\brief Data class containing relevant variables. To be used as template for task classes using it.
 *
 */
class HifstClientTaskData {
 public:
  HifstClientTaskData() :
    sidx ( 0 ),
    translation ( NULL ) {
  };

  /// Sentence index
  uint sidx;

  std::string sentence;
  ///Translated sentence will be stored here
  std::string *translation;
};

}
} // end namespaces

#endif

