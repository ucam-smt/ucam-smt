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
 * \brief Unit testing: Reference filter task
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

#include "tropical-sparse-tuple-weight.h"
#include "tropical-sparse-tuple-weight-decls.h"
#include "tropical-sparse-tuple-weight-funcs.h"

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-funcs.h"
#include "tropical-sparse-tuple-weight.makeweight.h"

#include "fstio.hpp"
#include "fstutils.hpp"
#include "fstutils.mapper.hpp"
#include "task.referencefilter.hpp"

using boost::any_cast;
namespace bfs = boost::filesystem;
namespace uh = ucam::hifst;

namespace googletesting {

/// Public class with variables required by TextTask to compile and run.
class DataForReferenceFilter {
 public:
  unsigned sidx;
  typedef fst::LexicographicArc< fst::StdArc::Weight, fst::StdArc::Weight> Arc;
  typedef fst::LexicographicWeight<fst::StdArc::Weight, fst::StdArc::Weight>
  Weight;

  std::vector < fst::VectorFst<Arc> *> filters;
  unordered_set<std::string> tvcb;
  unordered_map<std::string, fst::VectorFst<Arc> *> fsts;
};

///Basic test for ReferenceFilterTask class. Tests the whole pipeline.
TEST ( HifstReferenceFilter, basic_test ) {
  //Create test fst file expecto.fst.
  typedef fst::LexicographicArc< fst::StdArc::Weight, fst::StdArc::Weight> Arc;
  typedef fst::LexicographicWeight<fst::StdArc::Weight, fst::StdArc::Weight>
  Weight;
  fst::VectorFst<Arc> aux;
  fst::MakeWeight<Arc> mw;
  aux.AddState();
  aux.AddState();
  aux.AddState();
  aux.SetStart ( 0 );
  aux.SetFinal ( 2, Arc::Weight::One() );
  aux.AddArc ( 0, Arc ( 10, 10, mw ( 0 ), 1 ) );
  aux.AddArc ( 1, Arc ( 100, 100, mw ( 0 ), 2 ) );
  fst::FstWrite ( aux, "expecto.fst" );
  //Prepare RegistryPO object.
  unordered_map<std::string, boost::any> v;
  v[HifstConstants::kReferencefilterLoad] = std::string ( "expecto.fst" );
  v[HifstConstants::kReferencefilterWrite] = std::string ( "" );
  v[HifstConstants::kReferencefilterSubstring] = std::string ("yes");
  v[HifstConstants::kReferencefilterPrunereferenceweight] = float (
        std::numeric_limits<float>::max() );
  v[HifstConstants::kReferencefilterPrunereferenceshortestpath] = unsigned (
        std::numeric_limits<unsigned>::max() );
  v[HifstConstants::kReferencefilterLoadSemiring] = std::string("");
  v[HifstConstants::kHifstSemiring] = std::string("lexstdarc");
  DataForReferenceFilter d;
  d.sidx = 0;
  const uu::RegistryPO rg ( v );
  uh::ReferenceFilterTask<DataForReferenceFilter>  rft ( rg );
  EXPECT_EQ ( rft.getBuilt(), false );
  EXPECT_EQ ( rft.getDisableSubString(), false );
  EXPECT_EQ ( rft.getWeight(), std::numeric_limits<float>::max() );
  EXPECT_EQ ( rft.getShortestPath(), std::numeric_limits<unsigned>::max() );
  EXPECT_EQ ( rft.getVocabulary().size(), 0 );
  EXPECT_EQ ( rft.getTranslationLatticeFile(), "expecto.fst" );
  rft.run ( d );
  EXPECT_EQ ( rft.getBuilt(), true );
  ASSERT_EQ ( d.tvcb.size(), 2 );
  EXPECT_TRUE ( d.tvcb.find ( "10" ) != d.tvcb.end() );
  EXPECT_TRUE ( d.tvcb.find ( "100" ) != d.tvcb.end() );
  ASSERT_EQ ( d.filters.size(), 1 );
  EXPECT_EQ ( d.filters[0]->NumStates(), 3 );
  std::stringstream ss;
  fst::PrintFst ( *d.filters[0], &ss );
  EXPECT_EQ ( ss.str(),
              "0\t1\t10\t10\n0\t2\t100\t100\n0\n1\t2\t100\t100\n1\n2\n" );
  bfs::remove ( bfs::path ( "expecto.fst" ) );
};

///Basic test for ReferenceFilterTask class. Tests the whole pipeline.
TEST ( HifstReferenceFilter, empty ) {
  //Prepare RegistryPO object.
  unordered_map<std::string, boost::any> v;
  v[HifstConstants::kReferencefilterLoad] = std::string ( "" );
  v[HifstConstants::kReferencefilterWrite] = std::string ( "" );
  v[HifstConstants::kReferencefilterSubstring] = std::string ("yes");
  v[HifstConstants::kReferencefilterPrunereferenceweight] = float (
        std::numeric_limits<float>::max() );
  v[HifstConstants::kReferencefilterPrunereferenceshortestpath] = unsigned (
        std::numeric_limits<unsigned>::max() );
  v[HifstConstants::kReferencefilterLoadSemiring] = std::string("lexstdarc");
  v[HifstConstants::kHifstSemiring] = std::string("lexstdarc");
  DataForReferenceFilter d;
  d.sidx = 0;
  const uu::RegistryPO rg ( v );
  uh::ReferenceFilterTask<DataForReferenceFilter>  rft ( rg );
  rft.run ( d );
  EXPECT_EQ ( rft.getBuilt(), false );
  ASSERT_EQ ( d.filters.size(), 0 );
};

};

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
