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
 *    \brief Included headers for all the binary (hifst-client) should be defined here. This file should be included only once.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef HIFST_H
#define HIFST_H

namespace ucam {
namespace util {
extern bool user_check_ok;
extern const bool detailed;

}
} // end namespaces

///hifst-specific classes and methods included in this namespace.
#include "global_incls.hpp"
#include "custom_assert.hpp"
#include "global_decls.hpp"
#include "global_funcs.hpp"

#include "logger.hpp"

#include "szfstream.hpp"

#include "registrypo.hpp"
#include "taskinterface.hpp"
#include "range.hpp"
#include "addresshandler.hpp"
#include "multithreading.helpers.hpp"

#include <constants-fsttools.hpp>
#include <constants-hifst.hpp>

#include "main.hifst-client.init_param_options.hpp"
#include <data-main.hifst-client.hpp>

#endif
