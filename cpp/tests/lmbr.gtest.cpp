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
 * \brief Unit testing: String printing from lattice, multiepsilon composition, composition with failure transitions, generic weight mappers, multiple union of fsts, etc
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-funcs.h"

#include "range.hpp"
#include "fstio.hpp"
#include "fstutils.hpp"
#include "fstutils.mapper.hpp"
#include "fstutils.ftcompose.hpp"
#include "fstutils.extractngrams.hpp"

#include "taskinterface.hpp"

#include "data.lmbr.hpp"
#include "task.lmbr.hpp"

namespace bfs = boost::filesystem;

namespace ul = ucam::lmbr;

//Test the ngram extraction wrapper -- the test is pretty much the same as basic ngram extraction
TEST (lmbr, extractngrams) {
  fst::VectorFst<fst::StdArc> a, b;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( 1, 1, 0, 1 ) );
  a.AddState();
  a.AddArc ( 1, fst::StdArc ( 2, 2, 0, 2 ) );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 3, 3, 0, 3 ) );
  a.AddArc ( 1, fst::StdArc ( 5, 5, 0, 3 ) );
  a.AddState();
  a.AddArc ( 3, fst::StdArc ( 4, 4, 0, 4 ) );
  a.SetFinal ( 4, fst::StdArc::Weight::One() );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 6, 6, 0, 5 ) );
  a.AddArc ( 5, fst::StdArc ( 7, 7, 0, 4 ) );
  std::vector<fst::NGramList> ng;
  ul::extractNGrams<fst::StdArc> (a, ng);
  std::stringstream ss;
  for (unsigned k = 0; k < ng.size(); ++k)
    for (fst::NGramList::iterator itx = ng[k].begin(); itx != ng[k].end(); ++itx) {
      ss << itx->first << endl;
      EXPECT_TRUE (itx->second == fst::StdArc::Weight::One() );
    }
  std::string ngrams =
    "1\n2\n3\n4\n5\n6\n7\n6 7\n2 3\n1 5\n2 6\n1 2\n3 4\n5 4\n1 2 6\n1 5 4\n2 6 7\n2 3 4\n1 2 3\n1 2 3 4\n1 2 6 7\n";
  EXPECT_TRUE (ngrams == ss.str() );
  unsigned size = ul::extractNGrams<fst::StdArc> (b, ng);
  EXPECT_TRUE (size == 0);
}

TEST (lmbr, theta) {
  ul::Theta theta;
  EXPECT_EQ (theta (0), 0.0f);
  for (unsigned k = 1; k <= 4; ++k)
    EXPECT_EQ (theta (k), 1);
}

TEST (lmbr, computeapplyposteriors) {
  //Apply posteriors to a trivial hypotheses space and test scores.
  //Read evidence space = hypotheses space!
  fst::VectorFst<fst::StdArc> myfst;
  myfst.AddState();
  myfst.SetStart (0);
  myfst.AddState();
  myfst.AddArc (0, fst::StdArc (1, 1, 37.6542969, 1) );
  myfst.AddState();
  myfst.AddArc (1, fst::StdArc (933, 933, 0, 2) );
  myfst.AddState();
  myfst.AddArc (2, fst::StdArc (18, 18, 4.73828125, 3) );
  myfst.AddState();
  myfst.AddArc (2, fst::StdArc (150, 150, 0, 4) );
  myfst.AddState();
  myfst.AddArc (2, fst::StdArc (226, 226, 5.52148438, 5) );
  myfst.AddArc (2, fst::StdArc (508, 508, 8.22265625, 5) );
  myfst.AddArc (3, fst::StdArc (24, 24, 0, 5) );
  myfst.AddState();
  myfst.AddArc (5, fst::StdArc (150, 150, 0, 6) );
  myfst.AddState();
  myfst.AddArc (4, fst::StdArc (2, 2, 0, 7) );
  myfst.AddArc (4, fst::StdArc (23, 23, 5.90625, 7) );
  myfst.AddArc (6, fst::StdArc (2, 2, 0, 7) );
  myfst.SetFinal (7, fst::StdArc::Weight::One() );
  std::vector<fst::NGramList> ng;
  //Extract ngrams
  ul::extractNGrams<fst::StdArc> (myfst, ng);
  //Compute posteriors
  ul::ComputePosteriors cp (ng);
  cp (&myfst);
  {
    std::stringstream ss;
    uu::oszfstream j (ss);
    cp.WritePosteriors (j);
    std::string expectedposteriors =
      "0.002680170144\t23\n0.999996857723\t1\n0.000264338191\t508\n0.008617965440\t24\n0.997316613516\t2\n0.003937893375\t226\n0.999996857723\t150\n0.008617965440\t18\n0.999996857723\t933\n0.003937893375\t226 150\n0.997316613516\t150 2\n0.008617965440\t18 24\n0.003937893375\t933 226\n0.000264338191\t933 508\n0.987176612683\t933 150\n0.008617965440\t933 18\n0.999996857723\t1 933\n0.000264338191\t508 150\n0.008617965440\t24 150\n0.002680170144\t150 23\n0.000264338191\t933 508 150\n0.008617965440\t18 24 150\n0.984496437005\t933 150 2\n0.003937893375\t226 150 2\n0.987176612683\t1 933 150\n0.003937893375\t1 933 226\n0.000264338191\t1 933 508\n0.003937893375\t933 226 150\n0.008617965440\t24 150 2\n0.000264338191\t508 150 2\n0.008617965440\t933 18 24\n0.002680170144\t933 150 23\n0.008617965440\t1 933 18\n0.008617965440\t1 933 18 24\n0.008617965440\t933 18 24 150\n0.003937893375\t1 933 226 150\n0.008617965440\t18 24 150 2\n0.002680170144\t1 933 150 23\n0.003937893375\t933 226 150 2\n0.000264338191\t1 933 508 150\n0.000264338191\t933 508 150 2\n0.984496437005\t1 933 150 2\n";
    EXPECT_EQ (static_cast<std::stringstream *> (j.getStream() )->str(),
               expectedposteriors);
  }
  ul::Theta theta;
  ul::NGramToPosteriorsMapper& pst = cp.getPosteriors();
  ul::ApplyPosteriors ap (ng, pst, theta);
  fst::Map (&myfst, fst::RmWeightMapper<fst::StdArc>() );
  fst::VectorFst<fst::StdArc> *output = ap (myfst);
  myfst = *output;
  //Just check 1-best
  fst::ShortestPath (myfst, output);
  fst::TopSort (output);
  //  output->Write("output-final.fst");
  {
    std::stringstream ss;
    fst::printstrings (*output, &ss);
    EXPECT_EQ (ss.str(), "1 933 150 2 || 1 933 150 2 || -9.93797\n");
  }
  delete output;
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
