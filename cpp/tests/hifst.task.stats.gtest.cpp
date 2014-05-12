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
 * \brief Unit testing: Stats task testing
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
#include "task.stats.hpp"

using boost::any_cast;
namespace bfs = boost::filesystem;

namespace uf = ucam::fsttools;

namespace googletesting {

/// Public class with variables required by TextTask to compile and run.
struct DataForStats {
  DataForStats() :
    sidx ( 0 ),
    stats ( new uf::StatsData ) {
  }

  unsigned sidx;
  boost::scoped_ptr<uf::StatsData> stats;
};

///Basic test for stats task -- just write simple message and speed stats.
TEST ( HifstStatsTask, basic_test ) {
  //Prepare RegistryPO object.
  unordered_map<std::string, boost::any> v;
  const std::string kStatsText = "stats.text";
  v[HifstConstants::kStatsWrite] = std::string ( kStatsText );
  DataForStats d;
  //Hopefully these account for 0 time...
  d.stats->setTimeStart ( "sirius" );
  d.stats->setTimeEnd ( "sirius" );
  d.stats->setTimeStart ( "black" );
  d.stats->setTimeEnd ( "black" );
  const uu::RegistryPO rg ( v );
  {
    uf::SpeedStatsTask<DataForStats> st ( rg );
    st.run ( d );
  }
  uu::iszfstream aux ( kStatsText );
  std::string line;
  getline ( aux, line );
  getline ( aux, line );
  getline ( aux, line );
  boost::algorithm::trim ( line );
  EXPECT_EQ ( line.substr ( 0, 15 ), "sirius:        " );
  getline ( aux, line );
  boost::algorithm::trim ( line );
  EXPECT_EQ ( line.substr ( 0, 14 ), "black:        " );
  bfs::remove ( bfs::path ( kStatsText ) );
};

};

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
