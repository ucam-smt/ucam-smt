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

/** \file tests/hifst.task.grammar.gtest.cpp
 * \brief Unit testing: grammar task testing.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"
#include "defs.grammar.hpp"

#include "addresshandler.hpp"
#include "taskinterface.hpp"
#include "data.stats.hpp"
#include "data.grammar.hpp"
#include "task.grammar.hpp"

namespace uf = ucam::fsttools;
namespace uh = ucam::hifst;

///Trivial Data class with necessary variables for correct compilation
struct TaskData {
  TaskData() : stats ( new uf::StatsData ) {}
  uint sidx;
  uh::GrammarData *grammar;
  std::auto_ptr<uf::StatsData> stats;
};

/// Tests through stringstream  that the loading procedure is done correctly. TaskData is not really required for this test.
TEST ( HifstGrammar, task ) {
  uh::GrammarTask<TaskData> gt ( "", "" );
  std::stringstream ss;
  ss << "X 35 43 0" << endl << "S S_X S_X 0" << endl;
  gt.load ( ss );
  uh::GrammarData *grammar = gt.getGrammarData();
  ASSERT_EQ ( grammar->sizeofvpos, 2 );
  EXPECT_EQ ( grammar->vpos[1].p, 2 );
  EXPECT_EQ ( grammar->vpos[0].p, 12 );
}

TEST ( HifstGrammar, data_grammar ) {
  uh::GrammarTask<TaskData> gt ( "", "" );
  std::stringstream ss;
  ss << "X 35_47 43_55_58 0.45" << endl << "S S_X S_X 0.37" << endl;
  gt.load ( ss );
  uh::GrammarData *grammar = gt.getGrammarData();
  ASSERT_EQ ( grammar->vcat.size(), 2 );
  EXPECT_EQ ( grammar->vcat[1], "S" );
  EXPECT_EQ ( grammar->vcat[2], "X" );
  ASSERT_EQ ( grammar->categories.size(), 2 );
  EXPECT_EQ ( grammar->categories["S"], 1 );
  EXPECT_EQ ( grammar->categories["X"], 2 );
  EXPECT_EQ ( grammar->getRule ( 1 ), "X 35_47 43_55_58 0.45" );
  EXPECT_EQ ( grammar->getLHS ( 1 ), "X" );
  EXPECT_EQ ( grammar->getRHSSource ( 1 ), "35_47" );
  EXPECT_EQ ( grammar->getRHSSource ( 1, 0 ), "35" );
  EXPECT_EQ ( grammar->getRHSSource ( 1, 1 ), "47" );
  EXPECT_EQ ( grammar->getRHSSourceSize ( 1 ), 2 );
  EXPECT_EQ ( grammar->getRHSTranslation ( 1 ), "43_55_58" );
  EXPECT_EQ ( grammar->getRHSTranslationSize ( 1 ), 3 );
  ASSERT_EQ ( grammar->getRHSSplitTranslation ( 1 ).size(), 3 );
  EXPECT_EQ ( grammar->getRHSSplitTranslation ( 1 ) [0], "43" );
  EXPECT_EQ ( grammar->getRHSSplitTranslation ( 1 ) [1], "55" );
  EXPECT_EQ ( grammar->getRHSSplitTranslation ( 1 ) [2], "58" );
  EXPECT_EQ ( grammar->getIdx ( 1 ), 0 );
  EXPECT_EQ ( grammar->getRule ( 0 ), "S S_X S_X 0.37" );
  EXPECT_EQ ( grammar->getLHS ( 0 ), "S" );
  EXPECT_EQ ( grammar->getRHSSource ( 0 ), "S_X" );
  EXPECT_EQ ( grammar->getRHSSourceSize ( 0 ), 2 );
  EXPECT_EQ ( grammar->getRHSTranslation ( 0 ), "S_X" );
  EXPECT_EQ ( grammar->getRHSTranslationSize ( 0 ), 2 );
  ASSERT_EQ ( grammar->getRHSSplitTranslation ( 0 ).size(), 2 );
  EXPECT_EQ ( grammar->getRHSSplitTranslation ( 0 ) [0], "S" );
  EXPECT_EQ ( grammar->getRHSSplitTranslation ( 0 ) [1], "X" );
  EXPECT_EQ ( grammar->getIdx ( 0 ), 1 );
  EXPECT_EQ ( grammar->isPhrase ( 1 ), true );
  EXPECT_EQ ( grammar->isPhrase ( 0 ), false );
  EXPECT_EQ ( grammar->getWeight ( 0 ), 0.37f );
  EXPECT_EQ ( grammar->getWeight ( 1 ), 0.45f );
  unordered_map<uint, uint> mappings;
  grammar->getMappings ( 0, &mappings );
  EXPECT_EQ ( mappings.size(), 2 );
  mappings.clear();
  grammar->getMappings ( 1, &mappings );
  EXPECT_EQ ( mappings.size(), 0 );
}

///getSize function
TEST ( HifstGrammar, getSize ) {
  EXPECT_EQ ( uh::getSize ( "" ), 0 );
  EXPECT_EQ ( uh::getSize ( "35" ), 1 );
  EXPECT_EQ ( uh::getSize ( "35_47" ), 2 );
  EXPECT_EQ ( uh::getSize ( "35_M_47" ), 3 );
  EXPECT_EQ ( uh::getSize ( "35_M_47_X" ), 4 );
  EXPECT_EQ ( uh::getSize ( "35_M_47_X_2" ), 5 );
}

//Test isTerminal
TEST ( HifstGrammar, isTerminal ) {
  EXPECT_EQ ( uh::isTerminal ( "3" ), true );
  EXPECT_EQ ( uh::isTerminal ( "X" ), false );
}

//Test getFilteredNonTerminal
TEST ( HifstGrammar, getFilteredNonTerminal ) {
  using uh::getFilteredNonTerminal;
  std::string x = "X";
  getFilteredNonTerminal ( x );
  EXPECT_EQ ( x, "X" );
  x = "X2";
  getFilteredNonTerminal ( x );
  EXPECT_EQ ( x, "X" );
  x = "AB2";
  getFilteredNonTerminal ( x );
  EXPECT_EQ ( x, "AB" );
  x = "3";
  getFilteredNonTerminal ( x );
  EXPECT_EQ ( x, "3" );
}

//Test NonTerminalHierarchy
TEST ( HifstGrammar, nonterminalhierarchy ) {
  using uh::NonTerminalHierarchy;
  NonTerminalHierarchy x;
  x.insertIdentityRule ( "S X" );
  x.insertIdentityRule ( "X V" );
  x.insertIdentityRule ( "V W1" );
  x.insertIdentityRule ( "Z T" );
  std::string ntlist;
  EXPECT_EQ ( x ( ntlist ), true );
  EXPECT_EQ ( ntlist, "S,Z,T,X,V,W" );
  x.insertLHS ( "P" );
  x.insertLHS ( "Y" );
  EXPECT_EQ ( x ( ntlist ), true );
  EXPECT_EQ ( ntlist, "S,P,Y,Z,T,X,V,W" );
  NonTerminalHierarchy x2;
  x2.insertIdentityRule ( "S X" );
  x2.insertIdentityRule ( "X V" );
  x2.insertIdentityRule ( "V S" );
  EXPECT_EQ ( x2 ( ntlist ), false );
  EXPECT_EQ ( ntlist, "" );
  NonTerminalHierarchy x2b;
  x2b.insertIdentityRule ( "S X" );
  x2b.insertIdentityRule ( "X V" );
  x2b.insertIdentityRule ( "V X" );
  EXPECT_EQ ( x2b ( ntlist ), false );
  EXPECT_EQ ( ntlist, "" );
  NonTerminalHierarchy x3;
  EXPECT_EQ ( x3 ( ntlist ), true );
  EXPECT_EQ ( ntlist, "" );
  x3.insertLHS ( "P" );
  x3.insertLHS ( "A" );
  //Detects that S is not present
  EXPECT_EQ ( x3 ( ntlist ), false );
  EXPECT_EQ ( ntlist, "A,P" );
  x3.insertLHS ( "S" );
  EXPECT_EQ ( x3 ( ntlist ), true );
  EXPECT_EQ ( ntlist, "S,A,P" );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
