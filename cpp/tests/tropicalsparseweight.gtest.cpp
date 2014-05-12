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
 * \brief Unit testing: Tropical sparse tuple weight semiring
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"
#include "tropical-sparse-tuple-weight.h"
#include "tropical-sparse-tuple-weight-decls.h"
#include "tropical-sparse-tuple-weight-funcs.h"

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-funcs.h"
#include "tropical-sparse-tuple-weight.makeweight.h"

#include "fstutils.mapper.hpp"

namespace bfs = boost::filesystem;

//Convert a std weightto tropical sparse weight and back
TEST ( tropicalsparseweight, makesparseweight ) {
  fst::StdToVector<float> mw ( 1 );
  fst::StdArc::Weight stdweight ( 3.0f );
  fst::TropicalSparseTupleWeight<float>  w = mw ( stdweight );
  fst::VectorToStd<float> mw2;
  EXPECT_EQ ( mw2 ( w ).Value(), 3.0f );
};

///Generate tuple32 weight from float or another tuple32
TEST ( tropicalsparseweight, makeweight2 ) {
  fst::MakeWeight2<TupleArc32> mw;
  TupleArc32::Weight w = mw ( 3.0f );
  fst::VectorToStd<float> mw2;
  EXPECT_EQ ( mw2 ( w ).Value(), 3.0f );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
