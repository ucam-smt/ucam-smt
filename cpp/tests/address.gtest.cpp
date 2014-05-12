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
 * \brief Unit testing: integerpatternaddress
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>
#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "registrypo.hpp"
#include "addresshandler.hpp"

/**
 * \brief Tests wildcard expansion with an integer.
 */

TEST ( patternaddress, test1 ) {
  ucam::util::IntegerPatternAddress ipa ( "lm/?.lm.gz", "?" );
  std::string aux = ipa.get ( 3 );
  EXPECT_EQ ( aux, "lm/3.lm.gz" );
  aux = ipa ( 3 );
  EXPECT_EQ ( aux, "lm/3.lm.gz" );
  aux = ipa();
  EXPECT_EQ ( aux, "lm/?.lm.gz" );
}

/**
 * \brief Tests wildcard expansion with an integer when there is no wildcard to expand.
 */
TEST ( patternaddress, test2 ) {
  ucam::util::IntegerPatternAddress ipa ( "expecto.fst", "?" );
  std::string aux = ipa.get ( 3 );
  EXPECT_EQ ( aux, "expecto.fst" );
  aux = ipa ( 3 );
  EXPECT_EQ ( aux, "expecto.fst" );
  aux = ipa();
  EXPECT_EQ ( aux, "expecto.fst" );
}

/**
 * \brief Tests wildcard expansion with an integer when then string is empty.
 */

TEST ( patternaddress, test3 ) {
  ucam::util::IntegerPatternAddress ipa ( "", "?" );
  std::string aux = ipa.get ( 3 );
  EXPECT_EQ ( aux, "" );
  aux = ipa ( 3 );
  EXPECT_EQ ( aux, "" );
  aux = ipa();
  EXPECT_EQ ( aux, "" );
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
