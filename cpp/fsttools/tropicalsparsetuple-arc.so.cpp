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
 * \author Rory Waite
 * \brief Main target file for compilation into a shared library
 * \remark With these headers alone you can build a shared library
 * to use with openfst command-line tools -- will be able to handle lexicographic semiring.
 * Including these headers in your program enables it e.g. to read an fst directly with lexicographic semiring from file
 */

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "global_decls.hpp"
#include "params.hpp"
#include "tropical-sparse-tuple-weight-incls.h"
#include "tropical-sparse-tuple-weight.h"
#include "tropical-sparse-tuple-weight-decls.h"

using namespace fst;
using namespace fst::script;

REGISTER_FST_OPERATIONS ( TupleArc32 );
REGISTER_FST ( VectorFst, TupleArc32 );
REGISTER_FST ( ConstFst, TupleArc32 );
REGISTER_FST_CLASSES ( TupleArc32 );
