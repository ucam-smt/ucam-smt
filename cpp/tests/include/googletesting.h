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

#ifndef GOOGLETESTING_HPP
#define GOOGLETESTING_HPP

/** \file
 * \brief Unit testing: google testing common header
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#define TESTING
#define USER_CHECK_DEBUG


namespace ucam { namespace util {
extern bool user_check_ok;
extern const bool detailed;
  }}
#include <gtest/gtest.h>

#include "global_incls.hpp"
#include "global_decls.hpp"

#include "custom_assert.hpp"

#include "logger.hpp"
#include "global_funcs.hpp"

#include "szfstream.hpp"
#include "registrypo.hpp"
#include "range.hpp"

#include <constants-fsttools.hpp>
#include <constants-hifst.hpp>

inline void ucam::util::init_param_options ( int argc, const char* argv[], boost::program_options::variables_map *vm ) {};

#define KENLM_MAX_ORDER 6

namespace uu = ucam::util;


#endif
