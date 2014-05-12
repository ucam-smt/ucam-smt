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
 * \brief Unit testing: General functions
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"

///Test toString<unsigned>
TEST ( numberconversions, int2string ) {
  std::string y = ucam::util::toString<unsigned> ( 3 );
  EXPECT_EQ ( y, "3" );
}

///Test toNumber<unsigned>
TEST ( numberconversions, string2int ) {
  using ucam::util::toNumber;
  unsigned t = toNumber<unsigned> ( "35" );
  EXPECT_EQ ( t, 35 );
  user_check_ok = true;
  t = toNumber<unsigned> ( "35ab" );
  EXPECT_EQ ( t, 35 ); //Partially good result.
  EXPECT_EQ ( user_check_ok, false ); //User check has been triggered.
  user_check_ok = true;
  t = toNumber<unsigned> ( "ab" );
  EXPECT_EQ ( user_check_ok, false ); //User check should have been triggered.
  user_check_ok = true;
}

///Test countneedles
TEST ( stringutil, countneedles ) {
  using ucam::util::count_needles;
  std::string y = "3_5_7";
  EXPECT_EQ ( count_needles ( y, '_', 0, y.size() ), 2 );
  std::string y2 = "3";
  EXPECT_EQ ( count_needles ( y2, '_', 0, y2.size() ), 0 );
  std::string y3 = "";
  EXPECT_EQ ( count_needles ( y3, '_', 0, y3.size() ), 0 );
  std::string y4 = "__3__";
  EXPECT_EQ ( count_needles ( y4, '_', 0, y4.size() ), 4 );
}

///Test trim_spaces
TEST ( stringutil, trim_spaces ) {
  using ucam::util::trim_spaces;
  std::string input = " a b  c d";
  std::string reference = "a b c d";
  std::string output;
  trim_spaces ( input, &output );
  EXPECT_EQ ( output, reference );
  ///Empty case
  input = "";
  reference = "";
  trim_spaces ( input, &output );
  EXPECT_EQ ( output, reference );
};

///Test validate_source_sentence
TEST ( stringutil, validate_source_sentence ) {
  using ucam::util::validate_source_sentence;
  std::string input1 = " a b  c d";
  std::string input2 = " 1 2  3 4";
  std::string input3 = "";
  std::string input4 = "1 2 3 4";
  std::string input5 = "567";
  EXPECT_EQ ( validate_source_sentence ( input1 ), false );
  EXPECT_EQ ( validate_source_sentence ( input2 ), false );
  EXPECT_EQ ( validate_source_sentence ( input3 ), false );
  EXPECT_EQ ( validate_source_sentence ( input4 ), true );
  EXPECT_EQ ( validate_source_sentence ( input5 ), true );
}

///Dot product test

TEST ( dotproduct, basic_test ) {
  using ucam::util::dotproduct;
  std::vector<float> v1;
  std::vector<float> v2;
  v1.push_back ( 1 );
  v2.push_back ( 5 );
  EXPECT_EQ ( dotproduct ( v1, v2 ), 5 );
  v1.push_back ( 10 );
  v2.push_back ( 50 );
  EXPECT_EQ ( dotproduct ( v1, v2 ), 505 );
  v1.push_back ( 9 );
  EXPECT_EQ ( dotproduct ( v1, v2 ), 505 );
  v2.push_back ( 1000 );
  EXPECT_EQ ( dotproduct ( v1, v2 ), 9505 );
}

/// ParseParamString test
TEST ( stringutil, parseparamstring ) {
  using ucam::util::ParseParamString;
  std::string rule = "X 35 26 35.2 43.7 86.3";
  std::vector<float> v = ParseParamString<float> ( rule, 8 );
  ASSERT_EQ ( v.size(), 3 );
  EXPECT_EQ ( v[0], 35.2f );
  EXPECT_EQ ( v[1], 43.7f );
  EXPECT_EQ ( v[2], 86.3f );
  std::string rule2 = "X 35_47 43_55_58 0.45";
  std::vector<float> v2 = ParseParamString<float> ( rule2, 17 );
  ASSERT_EQ ( v2.size(), 1 );
  EXPECT_EQ ( v2[0], 0.45f );
}

/// trim_trailing_zeros tests. Reduce string representation of a float value
TEST ( stringutil, trim_trailing_zeros ) {
  using ucam::util::trim_trailing_zeros;
  std::string f = "0.370000";
  trim_trailing_zeros (f);
  EXPECT_EQ (f, "0.37");
  std::string f2 = "30";
  trim_trailing_zeros (f2);
  EXPECT_EQ (f2, "30");
  std::string f3 = "30.00";
  trim_trailing_zeros (f3);
  EXPECT_EQ (f3, "30");
}

TEST ( stringutil , ends_with ) {
  using ucam::util::ends_with;
  std::string s1 = "data/rules/trivial.grammar";
  std::string s2 = "";
  std::string s3 = "silly.gz";
  std::string s4 = "silly.gz.jopas";
  EXPECT_EQ (ends_with (s1, ".gz"), false);
  EXPECT_EQ (ends_with (s2, ".gz"), false);
  EXPECT_EQ (ends_with (s3, ".gz"), true);
  EXPECT_EQ (ends_with (s4, ".gz"), false);
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
