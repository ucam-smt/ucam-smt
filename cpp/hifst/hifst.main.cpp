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
 * \brief Hifst main entry file.
 * \date 8-12-2014
 * \author Gonzalo Iglesias
 */

///Include all necessary headers here.
#include <main.hifst.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.hifst.hpp>
#include <main-run.rules2weights.hpp>
#include <common-helpers.hpp>
#include <main.hpp>

/**
 * @brief Concrete RunTaskT implementation for Hifst tool.
 */
template < template <class> class DataT
           , class ArcT
           >
struct RunHifst {
  explicit RunHifst(ucam::util::RegistryPO const &rg){
  using ucam::hifst::SingleThreadedHifstTask;
  using ucam::hifst::MultiThreadedHifstTask;
  using ucam::hifst::HifstServerTask;
  using ucam::fsttools::RunTask3;
  (RunTask3<SingleThreadedHifstTask
          , MultiThreadedHifstTask
          , HifstServerTask
          , DataT
          , ArcT >
   (rg) );
  }
};

void ucam::util::MainClass::run() {
  using namespace HifstConstants;
  using namespace ::ucam::fsttools;
  std::string const arctype =rg_->get<std::string>(kHifstSemiring);
  using namespace ucam::hifst;
  if (arctype == kHifstSemiringLexStdArc) {
    (RunHifst<HifstTaskData, fst::LexStdArc>(*rg_));
  } else if (arctype == HifstConstants::kHifstSemiringTupleArc) {
    (RunHifst<HifstTaskData, TupleArc32>(*rg_));
    if (rg_->getBool(kRulesToWeightsEnable)) {
      ucam::hifst::SingleThreadededRulesToWeightsSparseLatsTask r2w(*rg_);
      r2w();
    }
  } else if (arctype == kHifstSemiringStdArc) {
    LWARN("Currently untested, might work in exact decoding:" << kHifstSemiringStdArc );
    (RunHifst<HifstTaskData, fst::StdArc>(*rg_));
  } else {
    LERROR("Unsupported semiring option");
    exit(EXIT_FAILURE);
  }
}
