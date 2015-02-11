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

#define HIFST

/**
 * \file
 * \brief Alignment lattices to sparse vector weight lattices
 * \date 15-12-2014
 * \author Gonzalo Iglesias
 */

///Include all necessary headers here.
#include <main.rules2weights.hpp>
#include <main-run.rules2weights.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <common-helpers.hpp>
#include <main.hpp>


// Note: The semiring is always Tuple32.
void ucam::util::MainClass::run() {
  ucam::hifst::SingleThreadededRulesToWeightsSparseLatsTask r2w(*rg_);
  r2w();
}
