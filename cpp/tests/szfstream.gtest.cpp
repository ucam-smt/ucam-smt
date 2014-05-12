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
 * \brief Unit testing: szfstream class for [file] operations
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

namespace bfs = boost::filesystem;

///Test iszfstream
TEST ( iszfstream, basictest ) {
  std::stringstream ss;
  ss << "expecto patronum" << endl << "obliviate" << endl << "avada kedavra";
  uu::iszfstream teststream ( ss );
  EXPECT_EQ ( teststream.is_open(), true );
  teststream.close();
  EXPECT_EQ ( teststream.is_open(), false );
  teststream.open ( ss );
  EXPECT_EQ ( teststream.is_open(), true );
  EXPECT_EQ ( teststream.eof(), false );
  std::string line;
  getline ( teststream, line );
  EXPECT_EQ ( line, "expecto patronum" );
  teststream >> line;
  EXPECT_EQ ( line, "obliviate" );
  getline ( teststream, line );
  EXPECT_EQ ( line, "" );
  getline ( teststream, line );
  EXPECT_EQ ( line, "avada kedavra" );
  EXPECT_EQ ( teststream.eof(), true );
}

TEST ( oszfstream, basictest ) {
  std::stringstream ss;
  uu::oszfstream teststream ( ss );
  EXPECT_EQ ( teststream.is_open(), true );
  teststream.close();
  EXPECT_EQ ( teststream.is_open(), false );
  teststream.open ( ss );
  EXPECT_EQ ( teststream.is_open(), true );
  std::string line = "obliviate";
  teststream << line << endl;
  std::string line2;
  * static_cast<std::stringstream *> ( teststream.getStream() ) >> line2;
  EXPECT_EQ ( line, line2 );
  teststream.close();
}

TEST (ioszfstream, combinedtest) {
  uu::oszfstream o ("test.gz");
  o << "expecto" << endl ;
  o << "patronum" << endl;
  o.close();
  uu::iszfstream i ("test.gz");
  std::string line;
  getline (i, line);
  EXPECT_EQ ("expecto", line);
  getline (i, line);
  EXPECT_EQ ("patronum", line);
  bfs::remove ( bfs::path ( "test.gz") );
}

///Basic test for FastForwardRead class.
TEST ( iszfstream, fastforwardread ) {
  //Do not delete!
  std::stringstream *ss = new std::stringstream;
  *ss << "one" << endl;
  *ss << "two" << endl;
  *ss << "three" << endl;
  *ss << "four" << endl;
  *ss << "five" ;
  {
    uu::FastForwardRead<> ffr ( ss );
    std::string line;
    bool finished;
    finished = ffr ( 2, &line );
    EXPECT_EQ ( line, "two" );
    EXPECT_EQ ( finished, false );
    finished = ffr ( 4, &line );
    EXPECT_EQ ( line, "four" );
    EXPECT_EQ ( finished, false );
    finished = ffr ( 5, &line );
    EXPECT_EQ ( line, "five" );
    EXPECT_EQ ( finished, false );
    finished = ffr ( 6, &line );
    //No more readings
    EXPECT_EQ ( line, "" );
    EXPECT_EQ ( finished, true );
    // Cannot read backwards
    user_check_ok = true;
    finished = ffr ( 1, &line );
    EXPECT_EQ ( line, "" );
    EXPECT_EQ ( user_check_ok, false );
    user_check_ok = true;
  }
  ss = new std::stringstream;
  *ss << "one" << endl;
  {
    std::string line;
    bool finished;
    uu::FastForwardRead<> ffr ( ss );
    finished = ffr ( 1, &line );
    EXPECT_EQ (finished, false);
    EXPECT_EQ (line, "one");
    finished = ffr ( 2, &line );
    EXPECT_EQ (finished, true);
    EXPECT_EQ (line, "");
  }
};

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
