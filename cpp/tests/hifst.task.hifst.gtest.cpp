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
 * \brief Unit testing: hifst lattice-building
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "params.hpp"
#include "wordmapper.hpp"

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
#include "fstutils.multiunion.hpp"

//KenLM
#include "lm/model.hh"
#include "fstutils.multiepsiloncompose.hpp"

#include "defs.grammar.hpp"
#include "defs.ssgrammar.hpp"
#include "defs.cykparser.hpp"

#include "data.stats.hpp"
#include "data.grammar.hpp"
#include "data.lm.hpp"

#include "fstutils.applylmonthefly.hpp"
#include "data.ssgrammar.hpp"
#include "data.cykparser.hpp"

#include "task.grammar.hpp"
#include "task.ssgrammar.hpp"
#include "task.cykparser.hpp"
#include "task.loadlm.hpp"
#include "task.hifst.hpp"
#include "data-main.hifst.hpp"

namespace bfs = boost::filesystem;
namespace uf = ucam::fsttools;
namespace uh =  ucam::hifst;

/**
 * \brief hifst test class for google test fixtures
 */
class HifstTest : public testing::Test {
 protected:  // You should make the members protected s.t. they can be
  // accessed from sub-classes.

  // virtual void SetUp() will be called before each test is run.  You
  // should define it if you need to initialize the variables.
  // Otherwise, this can be skipped.
  virtual void SetUp() {
    ///Initialize all variables required to use these task classes.
    using namespace HifstConstants;
    v_[kGrammarLoad] = std::string ( "" );
    v_[kGrammarStorepatterns] = std::string ( "" );
    v_[kGrammarStorentorder] = std::string ( "" );
    v_[kGrammarFeatureweights] = std::string ( "1" );
    v_[kSsgrammarStore] = std::string ( "" );
    v_[kSsgrammarAddoovsEnable] = std::string ("no");
    v_[kSsgrammarAddoovsSourcedeletions] = std::string ("no");
    v_[kCykparserHrmaxheight] = unsigned ( 10 );
    v_[kCykparserHmax] = std::string ( "X,10" );
    v_[kCykparserHmin] = std::string ( "X,0" );
    v_[kCykparserNtexceptionsmaxspan] = std::string ( "S" );
    v_[kHifstLatticeStore] = std::string ( "" );
    v_[kHifstLocalpruneEnable] = std::string ("no");
    v_[kHifstLocalpruneConditions] = std::string ( "" );
    v_[kHifstLocalpruneLmLoad] = std::string ("");
    v_[kHifstLocalpruneLmFeatureweights] = unsigned (1);
    v_[kHifstLocalpruneLmWordpenalty] = unsigned (0);
    v_[kHifstLocalpruneNumstates] = unsigned ( 1000 );
    v_[kHifstPrune] = float ( 1.0 );
    v_[kHifstUsepdt] = std::string ("no");
    v_[kHifstRtnopt] = std::string ("yes");
    v_[kHifstWritertn] = std::string ( "" );
    v_[kHifstAlilatsmode] = std::string ("no");
    v_[kHifstOptimizecells] = std::string ("yes");
    v_[kReferencefilterLoad] = std::string ("");
    const uu::RegistryPO rg ( v_ );
    //We need to generate some rules. It is easy to do so with GrammarTask, so we do it. We need to keep these tasks during all the lifespan of the tests, though.
    gt_ = new uh::GrammarTask<uh::HifstTaskData<> > ( rg );
    std::stringstream ss;
    ss << "X 3 3 0" << endl << "S S_X S_X 0" << endl;
    ss << "X 4 4 0" << endl << "X 5 5 0" << endl << "X 1 1 0" << endl << "X 2 2 0"
       << endl;
    ss << "X 3_4 3_4 0" << endl << "X 3_X_5 3_X_5 0" << endl << "S X X 0" << endl;
    gt_->load ( ss );
    d_.grammar = gt_->getGrammarData();
    for ( unsigned k = 0; k < 9; ++k ) {
      LINFO ( "RULE #" << k << ":" << d_.grammar->getRule ( k ) << ", real_idx=" <<
              d_.grammar->getIdx ( k ) );
    }
    // Now we prepare manually some instance-patterns
    d_.sidx = 0;
    d_.sentence = "1 3 4 5 2";
    d_.hpinstances["1"].push_back ( pair<unsigned, unsigned> ( 0, 0 ) );
    d_.hpinstances["3"].push_back ( pair<unsigned, unsigned> ( 1, 0 ) );
    d_.hpinstances["4"].push_back ( pair<unsigned, unsigned> ( 2, 0 ) );
    d_.hpinstances["5"].push_back ( pair<unsigned, unsigned> ( 3, 0 ) );
    d_.hpinstances["2"].push_back ( pair<unsigned, unsigned> ( 4, 0 ) );
    d_.hpinstances["3_4"].push_back ( pair<unsigned, unsigned> ( 1, 1 ) );
    d_.hpinstances["3_X_5"].push_back ( pair<unsigned, unsigned> ( 1, 2 ) );
    d_.hpinstances["X_X"].push_back ( pair<unsigned, unsigned> ( 0, 1 ) );
    d_.hpinstances["X"].push_back ( pair<unsigned, unsigned> ( 0, 0 ) );
    // Prepare sentence-specific grammar.
    ssgt_ = new uh::SentenceSpecificGrammarTask<uh::HifstTaskData<> > ( rg );
    ssgt_->run ( d_ );
    // Run cyk parse
    cyk_ = new uh::CYKParserTask<uh::HifstTaskData<> > ( rg );
    cyk_->run ( d_ );
    LINFO ( "Setup finished!" );
  }

  // virtual void TearDown() will be called after each test is run.
  // You should define it if there is cleanup work to do.  Otherwise,
  // you don't have to provide it.
  //
  virtual void TearDown() {
    LINFO ( "Teardown..." );
    delete gt_;
    delete ssgt_;
    delete cyk_;
  }

  // Declares the variables your tests want to use.
  uh::HifstTaskData<> d_;
  unordered_map<std::string, boost::any> v_;

  uh::GrammarTask<uh::HifstTaskData<> > *gt_;
  uh::SentenceSpecificGrammarTask<uh::HifstTaskData<> > *ssgt_;
  uh::CYKParserTask<uh::HifstTaskData<> > *cyk_;
};

/**
 *\brief Basic test for HifstTask
 */

TEST_F ( HifstTest, basic_translation ) {
  using namespace HifstConstants;
  v_[kHifstReplacefstbyarcNonterminals] = std::string ( "" );
  v_[kHifstReplacefstbyarcNumstates] = unsigned (
        std::numeric_limits<unsigned>::max() );
  v_[kHifstReplacefstbyarcExceptions] = std::string ( "" );
  const uu::RegistryPO rg ( v_ );
  LINFO ( "Number of inst:" << d_.hpinstances.size() );
  EXPECT_TRUE ( d_.cykdata != NULL );
  //Finally, build translation
  uh::HiFSTTask<uh::HifstTaskData<> > hifst ( rg );
  hifst.run ( d_ );
  //Test output.
  EXPECT_TRUE ( d_.fsts[kHifstLatticeStore] != NULL );
  std::stringstream ss;
  fst::printstrings ( *static_cast<fst::VectorFst<fst::LexStdArc> *>
                      (d_.fsts[kHifstLatticeStore]), &ss );
  EXPECT_EQ ( ss.str(),
              "1 3 4 5 2 || 1 3 4 5 2 || 0,0\n" );
};

/**
 *\brief Basic test for HifstTask
 */

TEST_F ( HifstTest, basic_translation2 ) {
  using namespace HifstConstants;
  v_[kHifstReplacefstbyarcNumstates] = unsigned (
        std::numeric_limits<unsigned>::max() );
  v_[kHifstReplacefstbyarcNonterminals] = std::string ( "X" );
  v_[kHifstReplacefstbyarcExceptions] = std::string ( "S" );
  const uu::RegistryPO rg ( v_ );
  LINFO ( "Number of inst:" << d_.hpinstances.size() );
  EXPECT_TRUE ( d_.cykdata != NULL );
  //Finally, build translation
  uh::HiFSTTask<uh::HifstTaskData<> > hifst ( rg );
  hifst.run ( d_ );
  //Test output.
  EXPECT_TRUE ( d_.fsts[kHifstLatticeStore] != NULL );
  std::stringstream ss;
  fst::printstrings ( * static_cast<fst::VectorFst<fst::LexStdArc> *>
                      (d_.fsts[kHifstLatticeStore]), &ss );
  EXPECT_EQ ( ss.str(),
              "1 3 4 5 2 || 1 3 4 5 2 || 0,0\n1 3 4 5 2 || 1 3 4 5 2 || 0,0\n1 3 4 5 2 || 1 3 4 5 2 || 0,0\n" );
};

/**
 *\brief Basic test for HifstTask
 */

TEST_F ( HifstTest, basic_translation3 ) {
  using namespace HifstConstants;
  {
    uu::oszfstream o ( "mylm" );
    o << endl;
    o << "\\data\\" << endl;
    o << "ngram 1=5" << endl;
    o << "ngram 2=2" << endl;
    o << "ngram 3=1" << endl;
    o << endl;
    o << "\\1-grams:" << endl;
    o << "-1\t3\t0" << endl;
    o << "-10\t4\t0" << endl;
    o << "-100\t5\t0" << endl;
    o << "-1000\t</s>\t0" << endl;
    o << "0\t<s>\t0" << endl;
    o << endl;
    o << "\\2-grams:" << endl;
    o << "-10000\t3 4\t0" << endl;
    o << "-100000\t4 </s>\t0" << endl;
    o << endl;
    o << "\\3-grams:" << endl;
    o << "-1000000\t3 4 </s>" << endl;
    o << endl;
    o << "\\end\\" << endl;
    o.close();
  }
  v_[kLmFeatureweights] = std::string ( "1.0" );
  v_[kLmLoad] = std::string ( "mylm" );
  v_[kLmWordmap] = std::string ("");
  v_[kHifstReplacefstbyarcNumstates] = unsigned (
        std::numeric_limits<unsigned>::max() );
  v_[kHifstReplacefstbyarcNonterminals] = std::string ( "X" );
  v_[kHifstReplacefstbyarcExceptions] = std::string ( "S" );
  const uu::RegistryPO rg ( v_ );
  LINFO ( "Number of inst:" << d_.hpinstances.size() );
  EXPECT_TRUE ( d_.cykdata != NULL );
  //Finally, build translation
  uf::LoadLanguageModelTask<uh::HifstTaskData<> > loadlm ( rg );
  uh::HiFSTTask<uh::HifstTaskData<> > hifst ( rg );
  loadlm.run ( d_ );
  hifst.run ( d_ );
  //Test output.
  EXPECT_TRUE ( d_.fsts[kHifstLatticeStore] != NULL );
  std::stringstream ss;
  fst::printstrings ( * static_cast<fst::VectorFst<fst::LexStdArc> *>
                      (d_.fsts[kHifstLatticeStore]), &ss );
  //Hypothesis <s> 3 4 5 </s> yields -11101
  //log2ln -11101=25560.99711732690113819501, apparently rounded somewhere to 25561.
  EXPECT_EQ ( ss.str(),
              "1 3 4 5 2 || 1 3 4 5 2 || 25561,0\n1 3 4 5 2 || 1 3 4 5 2 || 25561,0\n1 3 4 5 2 || 1 3 4 5 2 || 25561,0\n" );
  bfs::remove ( bfs::path ( "mylm" ) );
};

/**
 *\brief Basic test for HifstTask
 * \todo Test these options: cellredm,finalredm,deleteoov,usepdt,replacefstbyarc
 */

TEST_F ( HifstTest, alignment ) {
  using namespace HifstConstants;
  unordered_map<std::string, boost::any> v = v_;
  v[kHifstReplacefstbyarcNonterminals] = std::string ( "" );
  v[kHifstReplacefstbyarcNumstates] = unsigned (
        std::numeric_limits<unsigned>::max() );
  v[kHifstReplacefstbyarcExceptions] = std::string ( "S" );
  v[kHifstAlilatsmode] = std::string ("yes");
  const uu::RegistryPO rg ( v );
  LINFO ( "Number of inst:" << d_.hpinstances.size() );
  EXPECT_TRUE ( d_.cykdata != NULL );
  //Finally, build translation
  uh::HiFSTTask<uh::HifstTaskData<> > hifst ( rg );
  hifst.run ( d_ );
  //Test output.
  EXPECT_TRUE ( d_.fsts[kHifstLatticeStore] != NULL );
  /*
  std::stringstream ss;
  printstrings ( * static_cast<fst::VectorFst<fst::LexStdArc> *>(d_.fsts[kHifstLatticeStore]), &ss );

  EXPECT_EQ ( ss.str(),
                "5 9 1 2 3 2 4 2 6 2 || 1 3 4 5 2 || 0,0\n5 9 7 2 4 2 6 2 || 1 3 4 5 2 || 0,0\n5 9 3 8 2 6 2 || 1 3 4 5 2 || 0,0\n");
              //"5 9 7 2 4 2 6 2 || 1 3 4 5 2 || 0,0\n5 9 3 8 2 6 2 || 1 3 4 5 2 || 0,0\n5 9 1 2 3 2 4 2 6 2 || 1 3 4 5 2 || 0,0\n" );
  //  FstWrite(*d_.fsts[kHifstLatticeStore],"jajaja.fst");
        */
  fst::VectorFst<fst::LexStdArc> *a = new fst::VectorFst<fst::LexStdArc>;
  fst::VectorFst<fst::LexStdArc> *b = new fst::VectorFst<fst::LexStdArc>;
  fst::VectorFst<fst::LexStdArc> *c = new fst::VectorFst<fst::LexStdArc>;
  fst::string2fst<fst::LexStdArc> ("5 9 1 2 3 2 4 2 6 2", a);
  fst::string2fst<fst::LexStdArc> ("5 9 7 2 4 2 6 2", b);
  fst::string2fst<fst::LexStdArc> ("5 9 3 8 2 6 2", c);
  fst::MultiUnionReplace<fst::LexStdArc > mur;
  mur.Add (a);
  mur.Add (b);
  mur.Add (c);
  fst::VectorFst<fst::LexStdArc> *j = mur();
  fst::Determinize (fst::RmEpsilonFst<fst::LexStdArc> (*j), j);
  fst::Minimize (j);
  fst::RmEpsilon (j);
  fst::Project (static_cast<fst::VectorFst<fst::LexStdArc> *>
                (d_.fsts[kHifstLatticeStore]), fst::PROJECT_INPUT);
  fst::Determinize (fst::RmEpsilonFst<fst::LexStdArc>
                    (*static_cast<fst::VectorFst<fst::LexStdArc> *>
                     (d_.fsts[kHifstLatticeStore]) ),
                    static_cast<fst::VectorFst<fst::LexStdArc> *>
                    (d_.fsts[kHifstLatticeStore]) );
  fst::Minimize (static_cast<fst::VectorFst<fst::LexStdArc> *>
                 (d_.fsts[kHifstLatticeStore]) );
  fst::RmEpsilon (static_cast<fst::VectorFst<fst::LexStdArc> *>
                  (d_.fsts[kHifstLatticeStore]) );
  EXPECT_EQ (Equivalent (*static_cast<fst::VectorFst<fst::LexStdArc> *>
                         (d_.fsts[kHifstLatticeStore]), *j), true);
  delete j;
};

TEST ( HifstTest2, localconditions ) {

  using uh::conditions;
  uh::LocalPruningConditions lpc;
  ///e.g. Consider categories S=1,X=2,V=3,M=4
  /// The cyk cell is defined by (category,x,y)
  /// x is the position; for a sentence of n words, 0..n-1
  /// y is the word_span-1; for a sentence of n words, 0..n-1
  /// At each cell you will be building an FSA. The list of FSAs describe an RTN associated to the cyk grid.
  /// The expansion of this RTN should have 'numstates' states
  /// You want to know whether a particular cell lattice qualifies for local pruning:
  /// check (cc,y+1,numstates) and find out the weight.
  // Trigger pruning weight=9 if you are at or over cell (S,x,21) and your expanded cell lattice has +10000 states.
  lpc.add ( conditions ( 1, 20, 10000, 9.0f ) );
  // Trigger pruning weight=8.5 if you are at or over cell (X,x,11) and your expanded cell lattice has +1000 states.
  lpc.add ( conditions ( 2, 10, 1000, 8.5f ) );
  // Trigger pruning weight=10.5 if you are at or over cell (V,x,6) and your expanded cell lattice has +50 states.
  lpc.add ( conditions ( 3, 5, 50, 10.5f ) );
  // Trigger pruning weight=3.5 if you are at or over cell (V,x,6) and your expanded cell lattice has +500 states.
  lpc.add (conditions (3, 5 , 500 , 3.5f ) );
  float w = 0;
  ///No local pruning for category M...
  EXPECT_EQ ( lpc ( 4, 20, 10000, w ), false );
  EXPECT_EQ ( lpc ( 1, 18, 10000, w ), false );
  ///Only spanning 19 words...
  EXPECT_EQ ( lpc ( 1, 19, 10000, w ), false );
  EXPECT_EQ ( lpc ( 1, 20, 10000, w ), true );
  EXPECT_EQ ( w, 9.0 );
  EXPECT_EQ ( lpc ( 2, 10, 1000, w ), true );
  EXPECT_EQ ( w, 8.5 );
  EXPECT_EQ ( lpc ( 3, 5, 50, w ), true );
  EXPECT_EQ ( w, 10.5 );
  EXPECT_EQ ( lpc ( 3, 5, 499, w ), true );
  EXPECT_EQ ( w, 10.5 );
  EXPECT_EQ ( lpc ( 3, 5, 500, w ), true );
  EXPECT_EQ ( w, 3.5 );
  EXPECT_EQ ( lpc ( 3, 6, 500, w ), true );
  EXPECT_EQ ( w, 3.5 );
  EXPECT_EQ ( lpc ( 3, 6, 300, w ), true );
  EXPECT_EQ ( w, 10.5 );
  //Only 49 states
  EXPECT_EQ ( lpc ( 3, 5, 49, w ), false );
  //Below span=5
  EXPECT_EQ ( lpc ( 3, 1, 490, w ), false );
  //Unexpected entries
  EXPECT_EQ ( lpc ( 0, 0, 0, w ), false );
  EXPECT_EQ ( lpc ( std::numeric_limits<unsigned>::max(),
                    std::numeric_limits<unsigned>::max(), 0, w ), false );
  EXPECT_EQ ( lpc ( -1, -2, 0, w ), false );
  uh::LocalPruningConditions lpc2;
  //Given no conditions loaded, everything should be false!
  EXPECT_EQ ( lpc2 ( 3, 5, 49, w ), false );
};

///Testing the class ExpandedNumStatesRTN. Given a list of FSAs, it estimates the number of states of the equivalent expanded FSA.
TEST ( HifstTest2, expandednumstatesrtn ) {
  uh::ExpandedNumStatesRTN<fst::StdArc> test;
  fst::VectorFst<fst::StdArc> a, b;
  a.AddState();
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 2, fst::StdArc::Weight::One() );
  a.AddArc ( 0, fst::StdArc ( APBASETAG + 1 * APCCTAG + 1 * APXTAG + 1 * APYTAG,
                              APBASETAG + 1, 0, 1 ) );
  a.AddArc ( 1, fst::StdArc ( APBASETAG + 1 * APCCTAG + 1 * APXTAG + 1 * APYTAG,
                              APBASETAG + 1, 0, 2 ) );
  b.AddState();
  b.AddState();
  b.SetStart ( 0 );
  b.SetFinal ( 1, fst::StdArc::Weight::One() );
  b.AddArc ( 0, fst::StdArc ( 1, 1, 0, 1 ) );
  test.update ( 1, 1, 1, &b );
  test.update ( 1, 0, 1, &a );
  EXPECT_EQ ( test ( 1, 0, 1 ), 3 );
}

TEST ( HifstTest2, replacefstbyarc ) {
  uh::ReplaceFstByArc<fst::StdArc> x ( true, 4 );
  fst::VectorFst<fst::StdArc> a;
  for ( unsigned k = 0; k < 4; ++k ) a.AddState();
  fst::VectorFst<fst::StdArc> *aux = x ( a, 1 );
  ASSERT_TRUE ( aux == NULL );
  a.AddState();
  aux = x ( a, 1 );
  ASSERT_TRUE ( aux != NULL );
  EXPECT_EQ ( aux->NumStates(), 2 );
  ASSERT_EQ ( aux->NumArcs ( 0 ), 1 );
  delete aux;
}

TEST ( HifstTest2, manualreplacefstbyarc ) {
  uh::grammar_inversecategories_t vcat;
  vcat[1] = "S";
  vcat[2] = "X";
  unordered_set<std::string> replacefstbyarc;
  replacefstbyarc.insert ( "X" );
  unordered_set<std::string> replacefstbyarcexceptions;
  replacefstbyarcexceptions.insert ("S");
  fst::VectorFst<fst::StdArc> a;
  for ( unsigned k = 0; k < 2; ++k ) a.AddState();
  {
    uh::ManualReplaceFstByArc<fst::StdArc> x ( vcat, replacefstbyarc, true );
    fst::StdArc::Label label1 = APBASETAG + 1 * APCCTAG + 1;
    fst::VectorFst<fst::StdArc> *aux = x ( a, label1 );
    ASSERT_TRUE ( aux == NULL );
    fst::StdArc::Label label2 = APBASETAG + 2 * APCCTAG + 1;
    aux = x ( a, label2 );
    ASSERT_TRUE ( aux != NULL );
    EXPECT_EQ ( aux->NumStates(), 2 );
    delete aux;
  }
  ////Retest, now S will also be replaced
  {
    uh::ManualReplaceFstByArc<fst::StdArc> x ( vcat, replacefstbyarc, true, 0);
    fst::StdArc::Label label1 = APBASETAG + 1 * APCCTAG + 1;
    fst::VectorFst<fst::StdArc> *aux = x ( a, label1 );
    ASSERT_TRUE ( aux != NULL );
    EXPECT_EQ ( aux->NumStates(), 2 );
    delete aux;
    fst::StdArc::Label label2 = APBASETAG + 2 * APCCTAG + 1;
    aux = x ( a, label2 );
    ASSERT_TRUE ( aux != NULL );
    EXPECT_EQ ( aux->NumStates(), 2 );
    delete aux;
  }
  ////Retest, now S will also be replaced
  {
    uh::ManualReplaceFstByArc<fst::StdArc> x ( vcat, replacefstbyarc,
        replacefstbyarcexceptions, true, 0);
    fst::StdArc::Label label1 = APBASETAG + 1 * APCCTAG + 1;
    fst::VectorFst<fst::StdArc> *aux = x ( a, label1 );
    ASSERT_TRUE ( aux == NULL );
    fst::StdArc::Label label2 = APBASETAG + 2 * APCCTAG + 1;
    aux = x ( a, label2 );
    ASSERT_TRUE ( aux != NULL );
    EXPECT_EQ ( aux->NumStates(), 2 );
    delete aux;
  }
}

TEST ( HifstTest2, optimize ) {
  uh::OptimizeMachine<fst::StdArc> om;
  //FSA
  fst::VectorFst<fst::StdArc> a ;
  a.AddState();
  a.AddState();
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::StdArc::Weight::One() );
  a.SetFinal ( 3, fst::StdArc::Weight::One() );
  a.AddArc ( 0, fst::StdArc ( 1 , 1 , 0, 1 ) );
  a.AddArc ( 0, fst::StdArc ( 1 , 1 , 0, 2 ) );
  a.AddArc ( 2, fst::StdArc ( 0 , 0 , 0, 3 ) );
  fst::VectorFst<fst::StdArc> d (a);
  fst::RmEpsilon (&d); //If above numstatethreshold
  //If under numstatethreshold
  fst::VectorFst<fst::StdArc> e (a);
  fst::Determinize (fst::RmEpsilonFst<fst::StdArc> (e), &e);
  fst::Minimize (&e);
  fst::VectorFst<fst::StdArc> b (a);
  om (&b, 3);
  EXPECT_EQ ( d.NumStates(), b.NumStates() );
  b = a;
  om (&b, 4);
  EXPECT_TRUE ( Equivalent (e, b) );
  //Fst. Now we cannot use Equivalent easily.
  fst::VectorFst<fst::StdArc> c;
  c.AddState();
  c.AddState();
  c.AddState();
  c.AddState();
  c.SetStart ( 0 );
  c.SetFinal ( 1, fst::StdArc::Weight::One() );
  c.SetFinal ( 3, fst::StdArc::Weight::One() );
  c.AddArc ( 0, fst::StdArc ( 1 , 2 , 0, 1 ) );
  c.AddArc ( 0, fst::StdArc ( 1 , 2 , 0, 2 ) );
  c.AddArc ( 2, fst::StdArc ( 0 , 0 , 0, 3 ) );
  uh::OptimizeMachine<fst::StdArc> om1 (true);
  om1 (&c, 4);
  EXPECT_EQ (c.NumStates(), 2);
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
