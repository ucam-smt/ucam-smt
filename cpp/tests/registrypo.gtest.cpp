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
 * \brief Unit testing: registrypo class and related functions
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

namespace bfs = boost::filesystem;

///Test alternative constructor for testing
TEST ( registrypo, alternativeconstructor ) {
  unordered_map<std::string, boost::any> v;
  v["hobbits"] = 4;
  v["raistlin"] = std::string ( "shirak" );
  v["seoman"] = float ( 3.05f );
  uu::RegistryPO j ( v );
  EXPECT_EQ ( j.get<int> ( "hobbits" ), 4 );
  EXPECT_EQ ( j.get<std::string> ( "raistlin" ), "shirak" );
  EXPECT_EQ ( j.get<float> ( "seoman" ), 3.05f );
}

///Test dump method
TEST ( registrypo, dump ) {
  unordered_map<std::string, boost::any> v;
  v["hobbits"] = unsigned ( 4 );
  v["raistlin"] = std::string ( "shirak" );
  v["seoman"] = float ( 3.05f );
  uu::RegistryPO j ( v );
  EXPECT_EQ ( j.dump(),
              "\n\t\t+hobbits=4\n\t\t+raistlin=shirak\n\t\t+seoman=3.05\n\n" );
}

///Test registrypo get methods
TEST ( registrypo, alternativegetmethods ) {
  unordered_map<std::string, boost::any> v;
  v["goblin"] = std::string ( "S,X,V" );
  v["turin"] = std::string ( "X,1,Y,2" );
  v["turinwrong"] = std::string ( "X,A,Y,B" );
  v["turambar"] = std::string ( "" );
  v["smaug"] = std::string ( "T" );
  uu::RegistryPO j ( v );
  ASSERT_EQ ( j.getVectorString ( "goblin" ).size(), 3 );
  EXPECT_EQ ( j.getVectorString ( "goblin" ) [0], "S" );
  EXPECT_EQ ( j.getVectorString ( "goblin" ) [1], "X" );
  EXPECT_EQ ( j.getVectorString ( "goblin" ) [2], "V" );
  EXPECT_EQ ( j.getVectorString ( "goblin", 0 ) , "S" );
  EXPECT_EQ ( j.getVectorString ( "goblin", 1 ) , "X" );
  EXPECT_EQ ( j.getVectorString ( "goblin", 2 ) , "V" );
  ASSERT_EQ ( j.getMappedStringIndex ( "goblin" ).size(), 3 );
  EXPECT_EQ ( j.getMappedStringIndex ( "goblin" ) ["S"], 1 );
  EXPECT_EQ ( j.getMappedStringIndex ( "goblin" ) ["X"], 2 );
  EXPECT_EQ ( j.getMappedStringIndex ( "goblin" ) ["V"], 3 );
  ASSERT_EQ ( j.getMappedIndexString ( "goblin" ).size(), 3 );
  EXPECT_EQ ( j.getMappedIndexString ( "goblin" ) [1], "S" );
  EXPECT_EQ ( j.getMappedIndexString ( "goblin" ) [2], "X" );
  EXPECT_EQ ( j.getMappedIndexString ( "goblin" ) [3], "V" );
  ASSERT_EQ ( j.getPairMappedStringUInt ( "turin" ).size(), 2 );
  EXPECT_EQ ( j.getPairMappedStringUInt ( "turin" ) ["X"], 1 );
  EXPECT_EQ ( j.getPairMappedStringUInt ( "turin" ) ["Y"], 2 );
  user_check_ok = true;
  ASSERT_EQ ( j.getPairMappedStringUInt ( "turinwrong" ).size(), 2 );
  EXPECT_EQ ( user_check_ok, false );
  user_check_ok = true;
  std::unordered_set<std::string> aux = j.getSetString ( "goblin" );
  EXPECT_EQ ( aux.size(), 3 );
  EXPECT_TRUE ( aux.find ( "S" ) != aux.end() );
  EXPECT_TRUE ( aux.find ( "X" ) != aux.end() );
  EXPECT_TRUE ( aux.find ( "V" ) != aux.end() );
  ASSERT_EQ ( j.getVectorString ( "turambar" ).size(), 0 );
  ASSERT_EQ ( j.getMappedIndexString ( "turambar" ).size(), 0 );
  ASSERT_EQ ( j.getMappedStringIndex ( "turambar" ).size(), 0 );
  ASSERT_EQ ( j.getPairMappedStringUInt ( "turambar" ).size(), 0 );
  ASSERT_EQ ( j.getVectorString ( "smaug" ).size(), 1 );
  EXPECT_EQ ( j.getVectorString ( "smaug" ) [0], "T" );
}

///Test getstring method, which also reads from file if preceded by file://
TEST ( registrypo, get_string ) {
  unordered_map<std::string, boost::any> v;
  v["hobbits"] = std::string ( "file://hobbits" );
  uu::oszfstream o ( "hobbits" );
  o << "frodo,bilbo" << std::endl;
  o << "sam" << std::endl;
  o << "merry" << std::endl;
  o.close();
  uu::RegistryPO j ( v );
  EXPECT_EQ ( j.getString ( "hobbits" ), "frodo,bilbo sam merry" );
  bfs::remove ( bfs::path ( "hobbits" ) );
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
