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
 * \brief Unit testing: WordMapper, class that maps words to integers and vice versa
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "wordmapper.hpp"

///Basic test over WordMapper class.
TEST ( wordmapper , basic_test ) {
  std::stringstream ss;
  ss << "hey\t0\n";
  ss << "ho\t1\n";
  ss << "silver\t2\n";
  uu::iszfstream x ( ss );
  uu::WordMapper wm ( x, true );
  std::string mapped;
  wm ( "0 1 2", &mapped );
  EXPECT_EQ ( mapped, "hey ho silver" );
  wm ( "", &mapped );
  EXPECT_EQ ( mapped, "" );
  wm ( "hey ho silver", &mapped, true );
  EXPECT_EQ ( mapped, "0 1 2" );
  wm ( "", &mapped, true );
  EXPECT_EQ ( mapped, "" );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
