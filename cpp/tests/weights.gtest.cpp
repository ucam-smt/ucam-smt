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
 * \brief Unit testing: Various weight makers on different semirings
 * \date 11-11-2014
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"
#include "tropical-sparse-tuple-weight.h"
#include "tropical-sparse-tuple-weight-decls.h"
#include "tropical-sparse-tuple-weight-funcs.h"

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-funcs.h"
#include "tropical-sparse-tuple-weight.makeweight.h"

#include "fstutils.mapper.hpp"

#include "task.hifst.makeweights.hpp"

namespace bfs = boost::filesystem;

//Convert a std weightto tropical sparse weight and back
TEST ( tropicalsparseweight, makesparseweight ) {
  fst::StdToVector<float> mw ( 1 );
  fst::StdArc::Weight stdweight ( 3.0f );
  fst::TropicalSparseTupleWeight<float>  w = mw ( stdweight );
  fst::VectorToStd<float> mw2;
  EXPECT_EQ ( mw2 ( w ).Value(), 3.0f );
};

///Generate tuple32 weight from float or another tuple32
TEST ( tropicalsparseweight, makeweight2 ) {
  fst::MakeWeight2<TupleArc32> mw;
  TupleArc32::Weight w = mw ( 3.0f );
  fst::VectorToStd<float> mw2;
  EXPECT_EQ ( mw2 ( w ).Value(), 3.0f );
}

///Testing MakeWeight functor
TEST ( lexicographic, makeweight_stdarc ) {
  fst::MakeWeight<fst::StdArc> mw;
  fst::StdArc::Weight w = mw ( 3.0f );
  EXPECT_EQ ( 3.0f, w.Value() );
  fst::StdArc::Weight w2 = 4.0f;
  EXPECT_EQ ( 4.0f, mw ( w2 ) );
}

///Testing MakeWeight templated to lexstdarc
TEST ( lexicographic, makeweight_lexstdarc ) {
  fst::MakeWeight<fst::LexStdArc> mw;
  fst::LexStdArc::Weight w = mw ( 3.0f );
  EXPECT_EQ ( 3.0f, w.Value1() );
  EXPECT_EQ ( fst::StdArc::Weight::One(), w.Value2() );
  fst::LexStdArc::Weight w2 ( 6.5f, 7.0f );
  fst::LexStdArc::Weight w2check ( 6.5f, fst::StdArc::Weight::One() );
  EXPECT_EQ ( w2check, mw ( w2 ) );
}

///Testing MakeWeight2
TEST ( lexicographic, makeweight2_stdarc ) {
  fst::MakeWeight2<fst::StdArc> mw;
  fst::StdArc::Weight w = mw ( 2.0f );
  EXPECT_EQ ( 2.0f, w.Value() );
  fst::StdArc::Weight w2 = 4.5f;
  EXPECT_EQ ( 4.5f, mw ( w2 ) );
}

///Testing MakeWeight2 with LexStdArc
TEST ( lexicographic, makeweight2_lexstdarc ) {
  fst::MakeWeight2<fst::LexStdArc> mw;
  fst::LexStdArc::Weight w = mw ( 2.0f );
  EXPECT_EQ ( 2.0f, w.Value1() );
  EXPECT_EQ ( 2.0f, w.Value2() );
  fst::LexStdArc::Weight w2 ( 7.5f, 8.5f );
  fst::LexStdArc::Weight w2check ( 8.5f, 8.5f );
  EXPECT_EQ ( w2check, mw ( w2 ) );
};

///Test Map copy weight2 to weight1 -- should do nothing
TEST ( lexicographic, copyw2tow1_stdarc ) {
  fst::VectorFst<fst::StdArc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::StdArc::Weight::One() );
  fst::StdArc::Weight wa ( 3.0f );
  a.AddArc ( 0, fst::StdArc ( 1, 1, wa, 1 ) );
  //Should do nothing at all...
  fst::MakeWeight2<fst::StdArc> mw;
  fst::Map<fst::StdArc> ( &a,
                          fst::GenericWeightAutoMapper<fst::StdArc, fst::MakeWeight2<fst::StdArc> >
                          ( mw ) );
  fst::VectorFst<fst::StdArc> b;
  b.AddState();
  b.AddState();
  b.SetStart ( 0 );
  b.SetFinal ( 1, fst::StdArc::Weight::One() );
  fst::StdArc::Weight wb ( 3.0f );
  b.AddArc ( 0, fst::StdArc ( 1, 1, wb, 1 ) );
  EXPECT_TRUE ( Equivalent ( a, b ) );
}

///Test copy weight2 to weight1 -- effective on lexstdarc
TEST ( lexicographic, copyw2tow1_lexstdarc ) {
  fst::VectorFst<fst::LexStdArc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::LexStdWeight::One() );
  fst::LexStdWeight wa ( 3.0f, 2.0f );
  a.AddArc ( 0, fst::LexStdArc ( 1, 1, wa, 1 ) );
  fst::MakeWeight2<fst::LexStdArc> mw;
  fst::Map<fst::LexStdArc> ( &a,
                             fst::GenericWeightAutoMapper<fst::LexStdArc, fst::MakeWeight2<fst::LexStdArc> >
                             ( mw ) );
  fst::VectorFst<fst::LexStdArc> b;
  b.AddState();
  b.AddState();
  b.SetStart ( 0 );
  b.SetFinal ( 1, fst::LexStdWeight::One() );
  fst::LexStdWeight wb ( 2.0f, 2.0f );
  b.AddArc ( 0, fst::LexStdArc ( 1, 1, wb, 1 ) );
  EXPECT_TRUE ( Equivalent ( a, b ) );
}

// Test new hifst-tailored makeweights functor
// specialized for tropicalsparseweight.
TEST(tropicalsparseweight, makeweighthifst) {
  using ucam::hifst::MakeWeightHifst;
  using namespace HifstConstants;
  {
    unordered_map<std::string, boost::any> v;
    // Not disabling rule features:
    v[kHifstDisableRuleFeatures] = std::string ( "no" );
    // One language model with feature weight value 1.
    v[kLmFeatureweights] = std::string ( "1" );
    ucam::util::RegistryPO rg(v);
    MakeWeightHifst<TupleArc32> mwh(rg);
    TupleArc32::Weight w = mwh ( 3.0f );
    EXPECT_TRUE(ucam::util::toString<TupleArc32::Weight>(w, 0) == "0,1,2,3,");
    w = mwh ( 6.0f , 8 );
    EXPECT_TRUE(ucam::util::toString<TupleArc32::Weight>(w, 0) == "0,2,2,6,-10,1,");
  }
  {
    unordered_map<std::string, boost::any> v;
    v[kHifstDisableRuleFeatures] = std::string ( "yes" );
    // Two language models:
    v[kLmFeatureweights] = std::string ( "1,0.5" );
    ucam::util::RegistryPO rg(v);
    MakeWeightHifst<TupleArc32> mwh(rg);
    TupleArc32::Weight w = mwh ( 3.0f );
    EXPECT_TRUE(ucam::util::toString<TupleArc32::Weight>(w, 0) == "0,1,3,3,");
    w = mwh ( 6.0f , 8 );
    EXPECT_TRUE(ucam::util::toString<TupleArc32::Weight>(w, 0) == "0,1,3,6,");
  }
}
// In this context, the mapper should ignore, because we keep track separately of
// of the different language models.
// \todo: Hifst needs to be refactored in such a way that this mapper doesn't actually run.
// alternatively, we can consider emulating the lexicographic semiring

TEST(tropicalsparseweight, makeweighthifstMapper) {
  using namespace HifstConstants;

  typedef TupleArc32 Arc;
  typedef Arc::Weight Weight;
  fst::VectorFst<Arc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, Weight::One() );
  Weight wa;
  wa.Push(1, 1.0f);
  wa.Push(2, 2.0f);
  wa.Push(3, 3.0f);
  a.AddArc ( 0, Arc ( 1, 1, wa, 1 ) );
  unordered_map<std::string, boost::any> v;
  // irrelevant parameters for this test, but must be initialized.
  v[kHifstDisableRuleFeatures] = std::string ( "no" );
  // two language models:
  v[kLmFeatureweights] = std::string ( "1,0.5" );
  ucam::util::RegistryPO rg(v);
  ucam::hifst::MakeWeightHifst<Arc> mw(rg);
  fst::Map<Arc> ( &a,
                  fst::GenericWeightAutoMapper<Arc, ucam::hifst::MakeWeightHifst<Arc> >
                  ( mw ) );
  fst::VectorFst<Arc> b;
  b.AddState();
  b.AddState();
  b.SetStart ( 0 );
  b.SetFinal ( 1, Weight::One() );
  Weight wb;
  wb.Push(1, 1.0f);
  wb.Push(2, 2.0f);
  wb.Push(3, 3.0f);
  b.AddArc ( 0, Arc ( 1, 1, wb, 1 ) );
  EXPECT_TRUE ( Equivalent ( a, b ) );
}

TEST(lexicographic, makeweighthifst) {
  using ucam::hifst::MakeWeightHifst;
  unordered_map<std::string, boost::any> v;
  ucam::util::RegistryPO rg(v);
  MakeWeightHifst<fst::LexStdArc> mw(rg);
  fst::LexStdArc::Weight w = mw ( 2.0f );
  EXPECT_EQ ( 2.0f, w.Value1() );
  EXPECT_EQ ( 2.0f, w.Value2() );
  fst::LexStdArc::Weight w2 ( 7.5f, 8.5f );
  fst::LexStdArc::Weight w2check ( 8.5f, 8.5f );
  EXPECT_EQ ( w2check, mw ( w2 ) );
}

// @todo Test that it stores the language model score in
// a fixed position
// and it deletes scores on the same position when copied.
// TEST(tropicalsparseweight, makeweighthifstLocalLm) {
//   EXPECT_TRUE(0);
// }

// We need sparse tuple weight to do the same work as the lexicographic semiring
// this test simply shows the way these weights work.
// Although there are notes in the documentation suggesting
// that the list is sorted, this is not actually true.
// Also, indices can be repeated.
TEST(tropicalsparseweight, removeKthWeight) {
  using namespace fst;
  TupleArc32::Weight w,w2;
  w.Push(6,6);
  w.Push(5,5);
  w.Push(8,0);
  w.Push(5,-2);
  w.SetDefaultValue(300);
  std::stringstream ss; ss << w;
  EXPECT_EQ ( ss.str(), "300,3,6,6,5,5,5,-2,");
  // remove weight 5.

  // this does not copy default value
  for (SparseTupleWeightIterator<StdArc::Weight, int> it(w); !it.Done(); it.Next()) {
    if (it.Value().first != 5) // you cannot break;
      w2.Push(it.Value());
  }
  std::stringstream ss2; ss2 << w2;
  EXPECT_EQ ( ss2.str(), "0,1,6,6,");
  w2.SetDefaultValue(w.DefaultValue());
  std::stringstream ss3; ss3 << w2;
  EXPECT_EQ ( ss3.str(), "300,1,6,6,");

  // This is implemented in SparseTupleWeightMap (sparse-tuple-weight.h)
  // The default value is used for Times operation if not available.
  TupleArc32::Weight w3=Times(w,w2);
  //  std::cerr << w3 << std::endl;
  //  600,3,6,12,5,305,5,298
}





#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
