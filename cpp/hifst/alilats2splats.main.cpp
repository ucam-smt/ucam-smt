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
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

///Include all necessary headers here.
#include <main.alilats2splats.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.alilats2splats.hpp>
#include <common-helpers.hpp>
#include <main.hpp>

/**
 * @brief Concrete RunTaskT implementation for Hifst tool.
 */
template < template <class> class DataT
           , class ArcT
           >
struct RunAlilatsToSplats {
  explicit RunAlilatsToSplats(ucam::util::RegistryPO const &rg){
    using ucam::hifst::SingleThreadedAliLatsToSparseVecLatsTask;
    using ucam::hifst::MultiThreadedAliLatsToSparseVecLatsTask;
    using ucam::fsttools::RunTask2;
    (RunTask2<SingleThreadedAliLatsToSparseVecLatsTask
          , MultiThreadedAliLatsToSparseVecLatsTask
          , DataT
          , ArcT >
   (rg) );
  }
};

void ucam::util::MainClass::run() {
  using namespace HifstConstants;
  using namespace ucam::hifst;
  using namespace ucam::fsttools;

  (RunAlilatsToSplats<AlilatsToSparseWeightLatsData, TupleArc32> (*rg_));
}
