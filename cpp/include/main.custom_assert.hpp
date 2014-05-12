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
 * \brief Static variable for custom_assert. Include only once from main file.
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */

#ifndef CUSTOM_ASSERT_INIT
#define CUSTOM_ASSERT_INIT

#include "custom_assert.hpp"

#ifdef USER_CHECK_DEBUG
bool user_check_ok = true;
#endif

#endif
