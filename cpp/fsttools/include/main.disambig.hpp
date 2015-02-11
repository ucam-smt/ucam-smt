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

/** \file
 *  \brief Included headers for the binary (disambig) should be defined here. This file should be included only once.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef MAIN_DISAMBIG_H
#define MAIN_DISAMBIG_H

namespace ucam {
namespace util {
extern bool user_check_ok;
extern const bool detailed;
}
}

///hifst-specific classes and methods included in this namespace.
#include "global_incls.hpp"
#include "custom_assert.hpp"
#include "global_decls.hpp"
#include "global_funcs.hpp"

#include <fst/fstlib.h>

#include <fst/script/print.h>

#include "logger.hpp"

#include "szfstream.hpp"

#include "registrypo.hpp"
#include "taskinterface.hpp"
#include "range.hpp"
#include "addresshandler.hpp"

#include "multithreading.helpers.hpp"

#include <constants-fsttools.hpp>
#include "main.disambig.init_param_options.hpp"

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-decls.h"
#include "lexicographic-tropical-tropical-funcs.h"
#include <tropical-sparse-tuple-weight-incls.h>
#include <tropical-sparse-tuple-weight.h>
#include <tropical-sparse-tuple-weight-decls.h>
#include <tropical-sparse-tuple-weight.makeweight.h>

#include "fstio.hpp"
#include "fstutils.hpp"
#include "fstutils.ftcompose.hpp"

#define KENLM_MAX_ORDER 6
#include "lm/model.hh"

#include "data.stats.hpp"
#include "fstutils.mapper.hpp"
#include "fstutils.applylmonthefly.hpp"

#include "data.lm.hpp"
#include "data-main.disambig.hpp"

#include "task.readfst.hpp"
#include "task.writefst.hpp"
#include "task.loadlm.hpp"
#include "task.loadunimap.hpp"
#include "task.disambig.hpp"

#endif
