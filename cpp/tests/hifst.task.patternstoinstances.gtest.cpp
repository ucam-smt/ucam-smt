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
 * \brief Unit testing: converting grammar-specific patterns into instance patterns.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "addresshandler.hpp"
#include "taskinterface.hpp"

#include "data.stats.hpp"
#include "defs.grammar.hpp"
#include "data.grammar.hpp"
#include "task.patternstoinstances.hpp"

namespace uf = ucam::fsttools;
namespace uh = ucam::hifst;

/// Public Data class with variables required by PatternsToInstancestask to compile and run.
class DataForPatternsToInstancesTask {
 public:
  DataForPatternsToInstancesTask() :
    sidx (0),
    stats (new uf::StatsData) {
  };

  unsigned sidx;
  std::string sentence;
  uh::GrammarData *grammar;

  unordered_map<std::string, std::vector< std::pair <unsigned, unsigned> > >
  hpinstances;
  boost::shared_ptr<uf::StatsData> stats;
};

///Basic test for PatternsToInstancesTask class.
TEST ( HifstPatternsToInstances, basic_test ) {
  uh::GrammarData gd;
  DataForPatternsToInstancesTask d;
  d.grammar = &gd;
  ///Initialize all variables required to use task class
  unordered_map<std::string, boost::any> v;
  v[HifstConstants::kPatternstoinstancesMaxspan] = unsigned ( 5 );
  v[HifstConstants::kPatternstoinstancesGapmaxspan] = unsigned ( 2 );
  v[HifstConstants::kPatternstoinstancesStore] = std::string ( "" );
  const uu::RegistryPO rg ( v );
  uh::PatternsToInstancesTask<DataForPatternsToInstancesTask> ptask ( rg );
  gd.patterns.insert ( "w_X_w" );
  d.sentence = "1 3 4 5 2";
  ptask.instantiatePatternsHash ( d );
  EXPECT_EQ ( d.hpinstances.size(), 5 );
  EXPECT_FALSE ( d.hpinstances.find ( "1_X_2" ) != d.hpinstances.end() );
  EXPECT_TRUE ( d.hpinstances.find ( "1_X_4" ) != d.hpinstances.end() );
  EXPECT_TRUE ( d.hpinstances.find ( "1_X_5" ) != d.hpinstances.end() );
  EXPECT_TRUE ( d.hpinstances.find ( "3_X_2" ) != d.hpinstances.end() );
  EXPECT_TRUE ( d.hpinstances.find ( "3_X_5" ) != d.hpinstances.end() );
  EXPECT_TRUE ( d.hpinstances.find ( "4_X_2" ) != d.hpinstances.end() );
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
