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

/** \file tests/range.gtest.cpp
 * \brief Unit testing: getRange, InfiniteRange,IntegerRange, OneRange objects.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "range.hpp"

///Test getRange with 1:3
TEST ( range, sequence_of_integers1 ) {
  std::vector<unsigned> x;
  uu::getRange ( "1:3", x );
  ASSERT_EQ ( x.size(), 3 );
  EXPECT_EQ ( x[0], 1 );
  EXPECT_EQ ( x[1], 2 );
  EXPECT_EQ ( x[2], 3 );
}

///Test getRange (1:30:50)
TEST ( range, sequence_of_integers2 ) {
  std::vector<unsigned> x;
  uu::getRange ( "1:30:50", x );
  ASSERT_EQ ( x.size(), 2 );
  EXPECT_EQ ( x[0], 1 );
  EXPECT_EQ ( x[1], 31 );
}

///Test getRange (5,10,15:30:50,52)
TEST ( range, sequence_of_integers3 ) {
  std::vector<unsigned> x;
  uu::getRange ( "5,10,15:30:50,52", x );
  ASSERT_EQ ( x.size(), 5 );
  EXPECT_EQ ( x[0], 5 );
  EXPECT_EQ ( x[1], 10 );
  EXPECT_EQ ( x[2], 15 );
  EXPECT_EQ ( x[3], 45 );
  EXPECT_EQ ( x[4], 52 );
}

///Test getRange with 1:3
TEST ( range, sequence_of_floats ) {
  std::vector<float> x;
  uu::getRange ( "-1.0:0.5:1.0", x );
  ASSERT_EQ ( x.size(), 5 );
  EXPECT_EQ ( x[0], -1.0f );
  EXPECT_EQ ( x[1], -0.5f );
  EXPECT_EQ ( x[2], 0.0f );
  EXPECT_EQ ( x[3], 0.5f );
  EXPECT_EQ ( x[4], 1.0f );
}

///Test NumberRange<float>
TEST (range, floatrange) {
  std::vector<float> x;
  std::vector <float> result;
  uu::getRange ("5.3,10.2", x);
  uu::NumberRange<float> ir (x);
  for ( ir.start(); !ir.done(); ir.next() ) {
    result.push_back ( ir.get() );
  }
  ASSERT_EQ ( result.size(), 2 );
  EXPECT_EQ ( result[0], 5.3f );
  EXPECT_EQ ( result[1], 10.2f );
}

///Test NumberRange<unsigned>
TEST ( range, integerrange ) {
  std::vector<unsigned> x;
  std::vector<unsigned> result;
  uu::getRange ( "5,10", x );
  uu::NumberRange<unsigned> ir ( x );
  for ( ir.start(); !ir.done(); ir.next() ) {
    result.push_back ( ir.get() );
  }
  ASSERT_EQ ( result.size(), 2 );
  EXPECT_EQ ( result[0], 5 );
  EXPECT_EQ ( result[1], 10 );
}

///Test InfiniteRange
TEST ( range, infiniterange ) {
  uu::InfiniteRange<unsigned> x ( 5 );
  EXPECT_EQ ( x.done(), false );
  EXPECT_EQ ( x.get(), 5 );
  x.next();
  EXPECT_EQ ( x.get(), 6 );
  x.next();
  EXPECT_EQ ( x.get(), 7 );
}

///Test OneRange
TEST ( range, onerange ) {
  uu::OneRange<unsigned> x (5);
  EXPECT_EQ ( x.done(), false );
  x.start();
  EXPECT_EQ ( x.done(), false );
  EXPECT_EQ ( x.get(), 5 );
  x.next();
  EXPECT_EQ ( x.get(), 5 );
  EXPECT_EQ ( x.done(), true );
  x.next();
  EXPECT_EQ ( x.get(), 5 );
  EXPECT_EQ ( x.done(), true );
  x.start();
  EXPECT_EQ ( x.done(), false );
}

#ifndef GMAINTEST

/**
 * \brief main function.
 * If compiled individualy, will kickoff any tests in this file.
 */
int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
