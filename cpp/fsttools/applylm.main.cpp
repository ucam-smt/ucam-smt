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

/**
 * \file
 * \brief Main file for applylm tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <main.applylm.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.applylm.hpp>
#include <common-helpers.hpp>
#include <main.hpp>

/**
 * @brief Concrete RunTaskT implementation for applylm tool.
 */
template < template <class> class DataT
           , class ArcT
           >
struct RunApplyLm {
  explicit RunApplyLm(ucam::util::RegistryPO const &rg){
  using ucam::fsttools::RunTask2;
  using ucam::fsttools::SingleThreadedApplyLanguageModelTask;
  using ucam::fsttools::MultiThreadedApplyLanguageModelTask;
  (RunTask2<SingleThreadedApplyLanguageModelTask
          , MultiThreadedApplyLanguageModelTask
          , DataT
          , ArcT >
   (rg) );
  }
};

void ucam::util::MainClass::run() {
  using namespace HifstConstants;
  using ::ucam::fsttools::SingleThreadedApplyLanguageModelTask;
  using ::ucam::fsttools::MultiThreadedApplyLanguageModelTask;
  using ::ucam::fsttools::ApplyLMData;
  std::string const arctype =rg_->get<std::string>(kHifstSemiring);

  if (arctype == kHifstSemiringLexStdArc ) {
    (RunApplyLm<ApplyLMData, fst::LexStdArc>(*rg_));
  } else if (arctype == kHifstSemiringTupleArc) {
    LWARN("Untested, but might work:" << kHifstSemiringTupleArc );
    (RunApplyLm<ApplyLMData, TupleArc32>(*rg_));
  } else if (arctype == kHifstSemiringStdArc) {
    (RunApplyLm<ApplyLMData, fst::StdArc>(*rg_));
  } else {
    LERROR("Unsupported semiring option");
    exit(EXIT_FAILURE);
  }
}
