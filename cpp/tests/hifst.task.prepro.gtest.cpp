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
 * \brief Unit testing: Preprocess source test
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "addresshandler.hpp"
#include "taskinterface.hpp"

#include "fstutils.hpp"

#include "tokenizer.osr.hpp"
#include "wordmapper.hpp"

#include "task.prepro.hpp"
#include "data.stats.hpp"

using boost::any_cast;
namespace bfs = boost::filesystem;
namespace uh = ucam::hifst;
namespace uf = ucam::fsttools;

namespace googletesting {

struct PreProTaskData {
  PreProTaskData() :
    sidx ( 0 ),
    stats ( new uf::StatsData ) {
  }
  uint sidx;
  std::string originalsentence;
  std::string tokenizedsentence;
  std::string sentence;
  unordered_map<std::size_t, std::string> oovwmap;
  boost::scoped_ptr<uf::StatsData> stats;

  ///Wordmap/Integer map objects
  unordered_map<std::string, uu::WordMapper *> wm;

};

#ifndef OSR
///Basic test for prepro task
/// Tokenization not implemented.
TEST ( HifstPrePro, basic_test ) {
  unordered_map<std::string, boost::any> v;
  v[kPreproWordmapLoad] = std::string ( "" );
  v[kPreproTokenizeLanguage] = std::string ( "" );
  v[kPreproTokenizeEnable] = std::string ("no");
  const uu::RegistryPO rg ( v );
  uh::PreProTaskData d;
  d.originalsentence = "He's eating creamy creamy lovely potatoes.";
  stringstream ss;
  ss << "he\t0\n";
  ss << "'s\t1\n";
  ss << "eating\t2\n";
  ss << "potatoes\t3\n";
  ss << ".\t4\n";
  uu::iszfstream x ( ss );
  uu::WordMapper wm ( x, true );
  d.wm[kPreproWordmapLoad] = &wm;
  {
    uu::PreProTask<PreProTaskData> t ( rg );
    t.setTokenize ( true );
    t.run ( d );
  }
  EXPECT_EQ ( d.tokenizedsentence,
              "he 's eating creamy creamy lovely potatoes ." );
  EXPECT_EQ ( d.sentence, "0 1 2 " + toString ( OOVID ) + " " + toString (
                OOVID ) + " " + toString ( OOVID + 1 ) + " 3 4" );
  EXPECT_EQ ( d.oovwmap[OOVID], "creamy" );
  EXPECT_EQ ( d.oovwmap[OOVID + 1], "lovely" );
};

#endif

///Test to validate addSentenceMarkers
TEST ( stringutil, addsentencemarkers ) {
  std::string x = "a";
  uu::addSentenceMarkers ( x );
  EXPECT_EQ ( x, "<s> a </s>" );
  x = "";
  uu::addSentenceMarkers ( x );
  EXPECT_EQ ( x, "<s> </s>" );
  x = "    it is time   to fly  ";
  uu::addSentenceMarkers ( x );
  EXPECT_EQ ( x, "<s> it is time to fly </s>" );
}
};

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
