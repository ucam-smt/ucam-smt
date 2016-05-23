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
 * \brief Unit testing: sentence-specific grammar task.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"
#include "addresshandler.hpp"
#include "taskinterface.hpp"

#include "defs.grammar.hpp"
#include "defs.ssgrammar.hpp"

#include "data.stats.hpp"
#include "data.grammar.hpp"

#include "data.ssgrammar.hpp"
#include "task.grammar.hpp"

#include "task.ssgrammar.hpp"

namespace uh = ucam::hifst;
namespace uf = ucam::fsttools;

/// Public Data class with variables required by SentenceSpecificGrammarTask
struct DataForSentenceSpecificGrammarTask {
  DataForSentenceSpecificGrammarTask() :
    sidx ( 1 ),
    grammar ( NULL ),
    ssgd ( NULL ),
    stats ( new uf::StatsData ) {
  }

  unsigned sidx;
  std::string sentence;
  uh::GrammarData *grammar;
  unordered_map<std::string, std::vector< std::pair <unsigned, unsigned> > >
  hpinstances;
  uh::SentenceSpecificGrammarData *ssgd;
  std::auto_ptr<uf::StatsData> stats;
  std::unordered_set<std::string> tvcb;
};

///Basic test for TextTask class.
TEST ( HifstSentenceSpecificGrammarTask, basic_test ) {
  ///Initialize all variables required to uses these task classes.
  unordered_map<std::string, boost::any> v;
  v[HifstConstants::kGrammarFeatureweights] = std::string ( "1" );
  v[HifstConstants::kGrammarLoad] = std::string ( "" );
  v[HifstConstants::kGrammarStorepatterns] = std::string ( "" );
  v[HifstConstants::kGrammarStorentorder] = std::string ("");
  v[HifstConstants::kSsgrammarStore] = std::string ( "" );
  v[HifstConstants::kSsgrammarAddoovsEnable] = std::string ("yes");
  v[HifstConstants::kSsgrammarAddoovsSourcedeletions] = std::string ("no");
  const uu::RegistryPO rg ( v );
  //We need to generate some rules. It is easy to do so with GrammarTask, so we do it.
  boost::scoped_ptr< uh::GrammarTask<DataForSentenceSpecificGrammarTask> > gt (
    new  uh::GrammarTask<DataForSentenceSpecificGrammarTask> ( rg ) );
  std::stringstream ss;
  ss << "X 3 3 0" << std::endl << "S S_X S_X 0" << std::endl;
  ss << "X 4 4 0" << std::endl << "X 5 5 0" << std::endl;
  ss << "X 2 2 0" << std::endl;
  ss << "X 3_4 3_4 0" << std::endl << "X 3_X1_5 3_X1_5 0" << std::endl;
  //No rule for 1.
  //Instead, these two will apply on that word, as defined by grid instance pattern
  ss << "S X1 X1 0" << std::endl;
  ss << "S M1 M1 0" << std::endl;
  gt->load ( ss );
  boost::scoped_ptr< DataForSentenceSpecificGrammarTask> dor (
    new DataForSentenceSpecificGrammarTask );
  DataForSentenceSpecificGrammarTask& d = *dor;
  d.grammar = gt->getGrammarData();
  ASSERT_TRUE ( d.grammar != NULL );
  //Now insert patterns we want to allow, and where.
  d.grammar->patterns.insert ( "w" );
  d.grammar->patterns.insert ( "w_w" );
  d.grammar->patterns.insert ( "w_X_w" );
  d.grammar->patterns.insert ( "X_X" );
  d.sentence = "1 3 4 5 2 " + uu::toString ( OOVID );
  d.hpinstances["1"].push_back ( std::pair<unsigned, unsigned> ( 0, 0 ) );
  d.hpinstances["3"].push_back ( std::pair<unsigned, unsigned> ( 1, 0 ) );
  d.hpinstances["4"].push_back ( std::pair<unsigned, unsigned> ( 2, 0 ) );
  d.hpinstances["5"].push_back ( std::pair<unsigned, unsigned> ( 3, 0 ) );
  d.hpinstances["2"].push_back ( std::pair<unsigned, unsigned> ( 4, 0 ) );
  d.hpinstances[uu::toString ( OOVID )].push_back ( std::pair<unsigned, unsigned> ( 5,
      0 ) );
  d.hpinstances["3_4"].push_back ( std::pair<unsigned, unsigned> ( 1, 1 ) );
  d.hpinstances["3_X_5"].push_back ( std::pair<unsigned, unsigned> ( 1, 2 ) );
  d.hpinstances["X_X"].push_back ( std::pair<unsigned, unsigned> ( 1, 1 ) );
  //This one repeated on purpose. hpinstances are repeated if accepted for different spans, but ssgrammar shouldn't have repeated rules
  d.hpinstances["X_X"].push_back ( std::pair<unsigned, unsigned> ( 1, 1 ) );
  d.hpinstances["X"].push_back ( std::pair<unsigned, unsigned> ( 0, 0 ) );
  boost::scoped_ptr< uh::SentenceSpecificGrammarTask<DataForSentenceSpecificGrammarTask> >
  ssgt ( new uh::SentenceSpecificGrammarTask<DataForSentenceSpecificGrammarTask>
         ( rg ) );
  ssgt->run ( d );
  ASSERT_TRUE ( d.ssgd != NULL );
  EXPECT_EQ ( d.ssgd->rulesWithRhsSpan1.size(), 6 );
  std::vector<std::string> testrhs1, testrhs1X, testrhs1M;
  testrhs1.push_back ( "X 1 <oov> 0" );
  testrhs1.push_back ( "X 3 3 0" );
  testrhs1.push_back ( "X 4 4 0" );
  testrhs1.push_back ( "X 5 5 0" );
  testrhs1.push_back ( "X 2 2 0" );
  testrhs1.push_back ( "X " + uu::toString ( OOVID ) + " " + uu::toString (
                         OOVID ) + " 0" );
  testrhs1M.push_back ( "S M1 M1 0" );
  testrhs1X.push_back ( "S X1 X1 0" );
  for ( uh::ssgrammar_rulesmap_t::iterator itx =
          d.ssgd->rulesWithRhsSpan1.begin();
        itx != d.ssgd->rulesWithRhsSpan1.end();
        ++itx ) {
    ASSERT_TRUE ( itx->second.size() >= 1 );
    for ( uh::ssgrammar_firstelementmap_t::iterator itx2 = itx->second.begin();
          itx2 != itx->second.end(); ++itx2 ) {
      ASSERT_EQ ( itx->second[itx2->first].size(), 1 );
      std::string element = itx2->first;
      uh::getFilteredNonTerminal ( element );
      EXPECT_EQ ( element, itx2->first );
      //A bit hacky, but will suffice to check two rule candidates for the same position under the same pattern
      if ( itx2->first == "M" ) {
        EXPECT_EQ ( d.ssgd->getRule ( itx->second[itx2->first][0] ),
                    testrhs1M[itx->first] );
      } else if ( itx2->first == "X" ) {
        EXPECT_EQ ( d.ssgd->getRule ( itx->second[itx2->first][0] ),
                    testrhs1X[itx->first] );
      } else {
        //  cerr << "itx2->first=> " << itx2->first << "," << itx->second[itx2->first].size() << "," << "testrhs1=" << testrhs1[itx->first] <<  "," << d.ssgd->getRule ( itx->second[itx2->first][0] ) << endl;
        EXPECT_EQ ( d.ssgd->getRule ( itx->second[itx2->first][0] ),
                    testrhs1[itx->first] );
      }
    }
  }
  std::vector<std::string> testrhs2;
  testrhs2.push_back ( "X 3_4 3_4 0" );
  for ( uh::ssgrammar_rulesmap_t::iterator itx =
          d.ssgd->rulesWithRhsSpan2OrMore.begin();
        itx != d.ssgd->rulesWithRhsSpan2OrMore.end();
        ++itx ) {
    if ( itx->first != 1 ) ASSERT_TRUE ( !itx->second.size() );
    else {
      ASSERT_TRUE ( itx->second.size() == 2 );
    }
  }
  //Rules should only appear once per position. Example:
  EXPECT_EQ ( d.ssgd->rulesWithRhsSpan2OrMore[1]["S"].size(), 1 );
  //Testing existence of rules
  std::unordered_set<std::string> aux;
  aux.insert ( d.ssgd->getRule ( d.ssgd->rulesWithRhsSpan2OrMore[1]["S"][0] ) );
  aux.insert ( d.ssgd->getRule ( d.ssgd->rulesWithRhsSpan2OrMore[1]["3"][0] ) );
  aux.insert ( d.ssgd->getRule ( d.ssgd->rulesWithRhsSpan2OrMore[1]["3"][1] ) );
  EXPECT_TRUE ( aux.find ( "S S_X S_X 0" ) != aux.end() );
  EXPECT_TRUE ( aux.find ( "X 3_X1_5 3_X1_5 0" ) != aux.end() );
  EXPECT_TRUE ( aux.find ( "X 3_4 3_4 0" ) != aux.end() );
}

TEST ( HifstSentenceSpecificGrammarTask, data ) {
  uh::SentenceSpecificGrammarData gd ;
  uh::GrammarTask<DataForSentenceSpecificGrammarTask> gt ( "", "" );
  std::stringstream ss;
  ss << "XT 35_47_T T_43_55_58 0.450" << std::endl << "ST ST_XT ST_XT 0.370" << std::endl;
  gt.load ( ss );
  gd.grammar = gt.getGrammarData();
  gd.extrarules[0] = "S S_X S_X 0.37";
  gd.extrarules[1] = "X 35_47 43_55_58 0.45";
  EXPECT_EQ ( gd.getRule ( 1 ), "X 35_47 43_55_58 0.45" );
  EXPECT_EQ ( gd.getLHS ( 1 ), "X" );
  EXPECT_EQ ( gd.getRHSSource ( 1 ), "35_47" );
  EXPECT_EQ ( gd.getRHSSource ( 1, 0 ), "35" );
  EXPECT_EQ ( gd.getRHSSource ( 1, 1 ), "47" );
  EXPECT_EQ ( gd.getRHSSourceSize ( 1 ), 2 );
  EXPECT_EQ ( gd.getRHSTranslation ( 1 ), "43_55_58" );
  EXPECT_EQ ( gd.getRHSTranslationSize ( 1 ), 3 );
  ASSERT_EQ ( gd.getRHSSplitTranslation ( 1 ).size(), 3 );
  EXPECT_EQ ( gd.getRHSSplitTranslation ( 1 ) [0], "43" );
  EXPECT_EQ ( gd.getRHSSplitTranslation ( 1 ) [1], "55" );
  EXPECT_EQ ( gd.getRHSSplitTranslation ( 1 ) [2], "58" );
  EXPECT_EQ ( gd.getIdx ( 1 ), 1 );
  EXPECT_EQ ( gd.getRule ( 0 ), "S S_X S_X 0.37" );
  EXPECT_EQ ( gd.getLHS ( 0 ), "S" );
  EXPECT_EQ ( gd.getRHSSource ( 0 ), "S_X" );
  EXPECT_EQ ( gd.getRHSSourceSize ( 0 ), 2 );
  EXPECT_EQ ( gd.getRHSTranslation ( 0 ), "S_X" );
  EXPECT_EQ ( gd.getRHSTranslationSize ( 0 ), 2 );
  ASSERT_EQ ( gd.getRHSSplitTranslation ( 0 ).size(), 2 );
  EXPECT_EQ ( gd.getRHSSplitTranslation ( 0 ) [0], "S" );
  EXPECT_EQ ( gd.getRHSSplitTranslation ( 0 ) [1], "X" );
  EXPECT_EQ ( gd.getIdx ( 0 ), 0 );
  EXPECT_EQ ( gd.isPhrase ( 1 ), true );
  EXPECT_EQ ( gd.isPhrase ( 0 ), false );
  EXPECT_EQ ( gd.getWeight ( 0 ), 0.37f );
  EXPECT_EQ ( gd.getWeight ( 1 ), 0.45f );
  unordered_map<unsigned, unsigned> mappings;
  gd.getMappings ( 0, &mappings );
  EXPECT_EQ ( mappings.size(), 2 );
  mappings.clear();
  gd.getMappings ( 1, &mappings );
  EXPECT_EQ ( mappings.size(), 0 );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
