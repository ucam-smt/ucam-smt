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
 * \brief Unit testing: Postprocessing 1-best translation
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

#include "task.postpro.hpp"

using boost::any_cast;
namespace bfs = boost::filesystem;
namespace uh = ucam::hifst;

namespace googletesting {

struct PostProTaskData {
  PostProTaskData() :
    sidx ( 0 ),
    translation ( NULL ) {
  }
  uint sidx;
  unordered_map<std::size_t, std::string> oovwmap;
  unordered_map<std::string, fst::VectorFst<fst::StdArc> *> fsts;
  std::string *translation;

  ///Wordmap/Integer map objects
  unordered_map<std::string, uu::WordMapper *> wm;

};

#ifndef OSR
/// Basic test for PostProcessing: unmap to words and detokenize (english)
/// Skipped as detokenization not implemented
TEST ( HifstPostPro, basic_test ) {
  fst::VectorFst<fst::StdArc> aux;
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.SetStart ( 0 );
  aux.SetFinal ( 8, fst::StdArc::Weight::One() );
  aux.AddArc ( 0, fst::StdArc ( 1, 1, fst::StdArc::Weight ( 0 ), 1 ) );
  aux.AddArc ( 1, fst::StdArc ( 2, 2, fst::StdArc::Weight ( 0 ), 2 ) );
  aux.AddArc ( 2, fst::StdArc ( 3, 3, fst::StdArc::Weight ( 0 ), 3 ) );
  aux.AddArc ( 3, fst::StdArc ( OOVID, OOVID, fst::StdArc::Weight ( 0 ), 4 ) );
  aux.AddArc ( 4, fst::StdArc ( OOVID, OOVID, fst::StdArc::Weight ( 0 ), 5 ) );
  aux.AddArc ( 5, fst::StdArc ( OOVID + 1, OOVID + 1, fst::StdArc::Weight ( 0 ),
                                6 ) );
  aux.AddArc ( 6, fst::StdArc ( 4, 4, fst::StdArc::Weight ( 0 ), 7 ) );
  aux.AddArc ( 7, fst::StdArc ( 5, 5, fst::StdArc::Weight ( 0 ), 8 ) );
  std::stringstream ss;
  ss << "epsilon\t0\n";
  ss << "he\t1\n";
  ss << "'s\t2\n";
  ss << "eating\t3\n";
  ss << "potatoes\t4\n";
  ss << ".\t5\n";
  uu::iszfstream x ( ss );
  //Prepare RegistryPO object.
  unordered_map<std::string, boost::any> v;
  v[kPostproWordmapperLoad] = std::string ( "" );
  v[kPostproDetokenizeEnable] = std::string ("yes");
  v[kPostproDetokenizeLanguage] = std::string ( "" );
  const uu::RegistryPO rg ( v );
  uh::PostProTaskData d;
  d.translation = new std::string;
  d.oovwmap[OOVID] = "creamy";
  d.oovwmap[OOVID + 1] = "lovely";
  d.fsts["postpro.input"] = &aux;
  uu::WordMapper wm ( x );
  d.wm[kPostproWordmapLoad] = &wm;
  {
    uh::PostProTask<PostProTaskData> t ( rg );
    t.setDetokenize ( true );
    t.run ( d );
  }
  EXPECT_EQ ( *d.translation, "He's eating creamy creamy lovely potatoes." );
  delete d.translation;
};

#endif

///Testing function deleteSentenceMarkers
TEST ( hifstpostpro, deletesentencemarkers ) {
  std::string s = "1 3 2";
  uu::deleteSentenceMarkers ( s );
  EXPECT_EQ ( s, "3" );
  s = "<s> hola </s>";
  uu::deleteSentenceMarkers ( s );
  EXPECT_EQ ( s, "hola" );
  s = "<s> </s>";
  uu::deleteSentenceMarkers ( s );
  EXPECT_EQ ( s, "" );
  s = "1 2";
  uu::deleteSentenceMarkers ( s );
  EXPECT_EQ ( s, "" );
}

};

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
