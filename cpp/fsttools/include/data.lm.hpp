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

#ifndef DATA_LM_HPP
#define DATA_LM_HPP

/**
 * \file
 * \brief Implementation of a language model data structure using kenlm
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <wordmapper.hpp>
#include <idbridge.hpp>

namespace ucam {
namespace fsttools {

/**
 * \brief Language Model data structure
 */

struct KenLMData {
  KenLMData() :
    model ( NULL ),
    lmscale ( 1.0f ),
    lmwp (0.0f),
    wm (NULL) {
  };

  ///KenLM
  lm::base::Model * model;
  // Pointer to target grammar wordmap, if provided.
  ucam::util::WordMapper *wm;
  // map from target grammar ids to kenlm ids.
  IdBridge idb;
  ///Scales applied to each model
  float lmscale;
  float lmwp;
};

}
} // end namespaces

#endif
