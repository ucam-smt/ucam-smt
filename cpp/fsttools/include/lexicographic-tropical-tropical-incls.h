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

#ifndef LEXICOGRAPHIC_TROPICAL_TROPICAL_INCLS_H
#define LEXICOGRAPHIC_TROPICAL_TROPICAL_INCLS_H

/**
 * \file
 * \brief Headers for standalone shared library
 * \date October 2012
 * \author Gonzalo Iglesias
 */

#include <fst/arc.h>
#include <fst/vector-fst.h>
#include <fst/const-fst.h>
#include <fst/register.h>
#include <fst/script/fstscript.h>
#include <fst/script/fst-class.h>
#include <fst/script/register.h>
#include <fst/fst-decl.h>

namespace fst {

typedef LexicographicArc< StdArc::Weight, StdArc::Weight> LexStdArc;
typedef LexStdArc::Weight LexStdWeight;

};

#endif
