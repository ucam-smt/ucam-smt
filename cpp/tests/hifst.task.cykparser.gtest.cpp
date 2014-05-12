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
 * \brief Unit testing: cyk parser
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"
#include "wordmapper.hpp"

#include "addresshandler.hpp"
#include "taskinterface.hpp"

#include "defs.grammar.hpp"
#include "defs.ssgrammar.hpp"
#include "defs.cykparser.hpp"

#include "data.stats.hpp"
#include "data.grammar.hpp"
#include "data.ssgrammar.hpp"
#include "data.cykparser.hpp"

#include "task.grammar.hpp"
#include "task.ssgrammar.hpp"
#include "task.cykparser.hpp"

namespace uh = ucam::hifst;
namespace uf = ucam::fsttools;
namespace uu = ucam::util;

/// Public Data class with variables required by CyKParser
struct DataForCyK {
  DataForCyK() :
    sidx ( 0 ),
    grammar ( NULL ),
    ssgd ( NULL ),
    cykdata ( NULL ),
    stats ( new uf::StatsData ) {
  }

  unsigned sidx;
  std::string sentence;
  uh::GrammarData *grammar;

  ///Target vocabulary
  unordered_set<std::string> tvcb;

  unordered_map<std::string, std::vector< pair <unsigned, unsigned> > >
  hpinstances;
  uh::SentenceSpecificGrammarData *ssgd;
  uh::CYKdata *cykdata;
  boost::scoped_ptr<uf::StatsData> stats;
  unordered_map<std::string, uu::WordMapper *> wm;

};

/**
 *\brief Basic test for CykParserTask
 *
 */

TEST ( HifstCykParserTask, basic_test ) {
  ///Initialize all variables required to use these task classes.
  unordered_map<std::string, boost::any> v;
  v[HifstConstants::kGrammarLoad] = std::string ( "" );
  v[HifstConstants::kGrammarFeatureweights] = std::string ( "1" );
  v[HifstConstants::kGrammarStorepatterns] = std::string ( "" );
  v[HifstConstants::kGrammarStorentorder] = std::string ("");
  v[HifstConstants::kSsgrammarStore] = std::string ( "" );
  v[HifstConstants::kSsgrammarAddoovsEnable] = std::string ("no");
  v[HifstConstants::kSsgrammarAddoovsSourcedeletions] = std::string ("no");
  v[HifstConstants::kCykparserHrmaxheight] = unsigned ( 10 );
  v[HifstConstants::kCykparserHmax] = std::string ( "X,10" );
  v[HifstConstants::kCykparserHmin] = std::string ( "X,0" );
  v[HifstConstants::kCykparserNtexceptionsmaxspan] = std::string ( "S" );
  const uu::RegistryPO rg ( v );
  //We need to generate some rules. It is easy to do so with GrammarTask, so we do it.
  uh::GrammarTask<DataForCyK> gt ( rg );
  std::stringstream ss;
  ss << "X 3 3 0" << endl << "S S_X S_X 0" << endl;
  ss << "X 4 4 0" << endl << "X 5 5 0" << endl << "X 1 1 0" << endl << "X 2 2 0"
     << endl;
  ss << "X 3_4 3_4 0" << endl << "X 3_X1_5 3_X1_5 0" << endl << "S X1 X1 0" <<
     endl;
  gt.load ( ss );
  DataForCyK d;
  d.grammar = gt.getGrammarData();
  d.sentence = "1 3 4 5 2";
  d.hpinstances["1"].push_back ( pair<unsigned, unsigned> ( 0, 0 ) );
  d.hpinstances["3"].push_back ( pair<unsigned, unsigned> ( 1, 0 ) );
  d.hpinstances["4"].push_back ( pair<unsigned, unsigned> ( 2, 0 ) );
  d.hpinstances["5"].push_back ( pair<unsigned, unsigned> ( 3, 0 ) );
  d.hpinstances["2"].push_back ( pair<unsigned, unsigned> ( 4, 0 ) );
  d.hpinstances["3_4"].push_back ( pair<unsigned, unsigned> ( 1, 1 ) );
  d.hpinstances["3_X_5"].push_back ( pair<unsigned, unsigned> ( 1, 2 ) );
  d.hpinstances["X_X"].push_back ( pair<unsigned, unsigned> ( 0, 1 ) );
  d.hpinstances["X"].push_back ( pair<unsigned, unsigned> ( 0, 0 ) );
  uh::SentenceSpecificGrammarTask<DataForCyK> ssgt ( rg );
  ssgt.run ( d );
  //At this point we are ready to run a cyk parse.
  uh::CYKParserTask<DataForCyK> cyk ( rg );
  EXPECT_TRUE ( d.cykdata == NULL );
  cyk.run ( d );
  ASSERT_FALSE ( d.cykdata == NULL );
  EXPECT_EQ ( cyk.getFinalResult(), 1 );
  //Testing the cykgrid.
  ASSERT_EQ ( d.cykdata->cykgrid ( 1, 0, 0 ).size(), 1 );
  EXPECT_EQ ( d.cykdata->cykgrid ( 1, 0, 0, 0 ), 1 );
  ASSERT_EQ ( d.cykdata->cykgrid ( 1, 0, 2 ).size(), 2 );
  EXPECT_EQ ( d.cykdata->cykgrid ( 1, 0, 2, 0 ), 0 );
  EXPECT_EQ ( d.cykdata->cykgrid ( 1, 0, 2, 1 ), 0 );
  ASSERT_EQ ( d.cykdata->cykgrid ( 2, 1, 2 ).size(), 1 );
  EXPECT_EQ ( d.cykdata->cykgrid ( 2, 1, 2, 0 ), 4 );
  ASSERT_EQ ( d.cykdata->cykgrid ( 2, 1, 1 ).size(), 1 );
  EXPECT_EQ ( d.cykdata->cykgrid ( 2, 1, 1, 0 ), 5 );
  //test the cyk backpointers?
}

TEST ( HifstCykParserTask, cykgridfunctor ) {
  uh::CYKgrid cyk;
  EXPECT_EQ ( cyk.size(), 0 );
  //e.g. Adding rule 1 at cell 1,0,2
  cyk.Add ( 1, 0, 2, 1 );
  cyk.Add ( 1, 0, 2, 2 );
  cyk.Add ( 1, 0, 2, 3 );
  cyk.Add ( 0, 0, 0, 10 );
  ASSERT_EQ ( cyk ( 1, 0, 2 ).size(), 3 );
  ASSERT_EQ ( cyk ( 0, 0, 0 ).size(), 1 );
  EXPECT_EQ ( cyk ( 1, 0, 2 ) [0], 1 );
  EXPECT_EQ ( cyk ( 1, 0, 2 ) [1], 2 );
  EXPECT_EQ ( cyk ( 1, 0, 2 ) [2], 3 );
  EXPECT_EQ ( cyk ( 0, 0, 0 ) [0], 10 );
  EXPECT_EQ ( cyk ( 1, 0, 2, 0 ), 1 );
  EXPECT_EQ ( cyk ( 1, 0, 2, 1 ), 2 );
  EXPECT_EQ ( cyk ( 1, 0, 2, 2 ), 3 );
  EXPECT_EQ ( cyk ( 0, 0, 0, 0 ), 10 );
  //Any other position should be empty for now, e.g.
  ASSERT_EQ ( cyk ( 2, 0, 3 ).size(), 0 );
  cyk.reset();
  EXPECT_EQ ( cyk.size(), 0 );
}

TEST ( HifstCykParserTask, cykbpfunctor ) {
  uh::CYKbackpointers bp;
  uh::cykparser_rulebpcoordinates_t aux;
  aux.push_back ( 1 );
  aux.push_back ( 1 );
  aux.push_back ( 1 );
  bp.Add ( 1, 0, 2, aux );
  aux.clear();
  aux.push_back ( 2 );
  aux.push_back ( 2 );
  aux.push_back ( 2 );
  bp.Add ( 1, 0, 2, aux );
  aux.clear();
  bp.Add ( 0, 0, 0, aux );
  ASSERT_EQ ( bp ( 1, 0, 2 ).size(), 2 );
  ASSERT_EQ ( bp ( 0, 0, 0 ).size(), 0 );
  uh::cykparser_ruledependencies_t aux2 = bp ( 1, 0, 2 );
  ASSERT_EQ ( aux2[0].size(), 3 );
  ASSERT_EQ ( aux2[1].size(), 3 );
  EXPECT_EQ ( aux2[0][0], 1 );
  EXPECT_EQ ( aux2[0][1], 1 );
  EXPECT_EQ ( aux2[0][2], 1 );
  EXPECT_EQ ( aux2[1][0], 2 );
  EXPECT_EQ ( aux2[1][1], 2 );
  EXPECT_EQ ( aux2[1][2], 2 );
  bp.reset();
  EXPECT_EQ ( bp.size(), 0 );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
