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
 * \brief Unit testing: lexicographic semiring
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-funcs.h"

#include "fstutils.mapper.hpp"

namespace bfs = boost::filesystem;

///Testing MakeWeight functor
TEST ( lexicographic, makeweight_stdarc ) {
  fst::MakeWeight<fst::StdArc> mw;
  fst::StdArc::Weight w = mw ( 3.0f );
  EXPECT_EQ ( 3.0f, w.Value() );
  fst::StdArc::Weight w2 = 4.0f;
  EXPECT_EQ ( 4.0f, mw ( w2 ) );
}

///Testing MakeWeight templated to lexstdarc
TEST ( lexicographic, makeweight_lexstdarc ) {
  fst::MakeWeight<fst::LexStdArc> mw;
  fst::LexStdArc::Weight w = mw ( 3.0f );
  EXPECT_EQ ( 3.0f, w.Value1() );
  EXPECT_EQ ( fst::StdArc::Weight::One(), w.Value2() );
  fst::LexStdArc::Weight w2 ( 6.5f, 7.0f );
  fst::LexStdArc::Weight w2check ( 6.5f, fst::StdArc::Weight::One() );
  EXPECT_EQ ( w2check, mw ( w2 ) );
}

///Testing MakeWeight2
TEST ( lexicographic, makeweight2_stdarc ) {
  fst::MakeWeight2<fst::StdArc> mw;
  fst::StdArc::Weight w = mw ( 2.0f );
  EXPECT_EQ ( 2.0f, w.Value() );
  fst::StdArc::Weight w2 = 4.5f;
  EXPECT_EQ ( 4.5f, mw ( w2 ) );
}

///Testing MakeWeight2 with LexStdArc
TEST ( lexicographic, makeweight2_lexstdarc ) {
  fst::MakeWeight2<fst::LexStdArc> mw;
  fst::LexStdArc::Weight w = mw ( 2.0f );
  EXPECT_EQ ( 2.0f, w.Value1() );
  EXPECT_EQ ( 2.0f, w.Value2() );
  fst::LexStdArc::Weight w2 ( 7.5f, 8.5f );
  fst::LexStdArc::Weight w2check ( 8.5f, 8.5f );
  EXPECT_EQ ( w2check, mw ( w2 ) );
};

///Test Map copy weight2 to weight1 -- should do nothing
TEST ( lexicographic, copyw2tow1_stdarc ) {
  fst::VectorFst<fst::StdArc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::StdArc::Weight::One() );
  fst::StdArc::Weight wa ( 3.0f );
  a.AddArc ( 0, fst::StdArc ( 1, 1, wa, 1 ) );
  //Should do nothing at all...
  fst::MakeWeight2<fst::StdArc> mw;
  fst::Map<fst::StdArc> ( &a,
                          fst::GenericWeightAutoMapper<fst::StdArc, fst::MakeWeight2<fst::StdArc> >
                          ( mw ) );
  fst::VectorFst<fst::StdArc> b;
  b.AddState();
  b.AddState();
  b.SetStart ( 0 );
  b.SetFinal ( 1, fst::StdArc::Weight::One() );
  fst::StdArc::Weight wb ( 3.0f );
  b.AddArc ( 0, fst::StdArc ( 1, 1, wb, 1 ) );
  EXPECT_TRUE ( Equivalent ( a, b ) );
}

///Test copy weight2 to weight1 -- effective on lexstdarc
TEST ( lexicographic, copyw2tow1_lexstdarc ) {
  fst::VectorFst<fst::LexStdArc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::LexStdWeight::One() );
  fst::LexStdWeight wa ( 3.0f, 2.0f );
  a.AddArc ( 0, fst::LexStdArc ( 1, 1, wa, 1 ) );
  fst::MakeWeight2<fst::LexStdArc> mw;
  fst::Map<fst::LexStdArc> ( &a,
                             fst::GenericWeightAutoMapper<fst::LexStdArc, fst::MakeWeight2<fst::LexStdArc> >
                             ( mw ) );
  fst::VectorFst<fst::LexStdArc> b;
  b.AddState();
  b.AddState();
  b.SetStart ( 0 );
  b.SetFinal ( 1, fst::LexStdWeight::One() );
  fst::LexStdWeight wb ( 2.0f, 2.0f );
  b.AddArc ( 0, fst::LexStdArc ( 1, 1, wb, 1 ) );
  EXPECT_TRUE ( Equivalent ( a, b ) );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
