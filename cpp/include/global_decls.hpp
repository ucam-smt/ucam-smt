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

/** \file include/global_decls.hpp
 * \brief General typedefs, defines...
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef GLOBAL_DECL_HPP
#define GLOBAL_DECL_HPP

#include <openfstversion.hpp>

#define OSR

#if OPENFSTVERSION >= 1002000
typedef int64_t
int64; //int64 is defined this way starting with openfst-1.2.x, but not in openfst-1.1
#else
typedef long long int64;  //int64 is defined this way up to openfst-1.1,...
#endif

#define APRULETAG 2000000000
#define APBASETAG 1000000000
#define APCCTAG 1000000
#define APXTAG 1000
#define APYTAG 1

//key words...
#define OOVID 100000000
#define DR  999999999
#define OOV 999999998
#define GAP 999999997
#define PHI 999999996
#define RHO 999999995
#define SIGMA 999999994
#define UNIQUE 999999993
#define SEP 999999992
#define PROTECTEDEPSILON 999999991

#define NORULE 0
#define EPSILON 0

#define HASH_MODULE 1000000

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define ZDISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#endif
