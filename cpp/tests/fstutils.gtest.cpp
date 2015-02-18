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

#include "lm/model.hh"

#include "lexicographic-tropical-tropical-incls.h"
#include "lexicographic-tropical-tropical-funcs.h"

#include "fstutils.hpp"
#include "fstutils.ftcompose.hpp"
#include "fstutils.multiepsiloncompose.hpp"
#include "fstutils.extractngrams.hpp"
#include "fstutils.applylmonthefly.hpp"
#include "fstutils.mapper.hpp"
#include "fstutils.multiunion.hpp"
#include "fstio.hpp"

#include <idbridge.hpp>
#include <hifst_enumerate_vocab.hpp>

namespace bfs = boost::filesystem;

//Testing recursive implementation of printstrings
TEST ( fstutils, printstrings ) {
  fst::VectorFst<fst::StdArc> a, b, c;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( DR, DR, 0.5, 1 ) );
  a.AddArc ( 0, fst::StdArc ( OOV, OOV, 1.5, 1 ) );
  a.AddState();
  a.AddArc ( 1, fst::StdArc ( OOV, OOV, 0.5, 2 ) );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 3, 3, 0.5, 3 ) );
  a.SetFinal ( 3, fst::StdArc::Weight::One() );
  fst::ArcSort ( &a, fst::OLabelCompare<fst::StdArc>() );
  {
    std::stringstream ss;
    printstrings ( a, &ss );
    EXPECT_EQ ( ss.str(),
                "999999998 999999998 3 || 999999998 999999998 3 || 2.5\n999999999 999999998 3 || 999999999 999999998 3 || 1.5\n" );
  }
  b.AddState();
  b.SetStart ( 0 );
  {
    std::stringstream ss;
    printstrings ( b, &ss );
    EXPECT_EQ ( ss.str(),
                "|| || 0\n" );
  }
  {
    std::stringstream ss;
    user_check_ok = true;
    printstrings ( c, &ss );  //Test with empty lattice -- will complain.
    EXPECT_EQ ( user_check_ok, false );
    EXPECT_EQ ( ss.str(), "" );
    user_check_ok = true;
  }
}

///Testing multiepsilon composition. You can use a matcher to say that in your world several labels should be treated as epsilons in this particular composition
///This should be equivalent to removing epsilons on the right side and composing
TEST ( fstutils, multiepsiloncomposition ) {
  fst::VectorFst<fst::StdArc> a, b;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( DR, DR, 0.5, 1 ) );
  a.AddArc ( 0, fst::StdArc ( OOV, OOV, 1.5, 1 ) );
  a.AddState();
  a.AddArc ( 1, fst::StdArc ( OOV, OOV, 0.5, 2 ) );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 3, 3, 0.5, 3 ) );
  a.SetFinal ( 3, fst::StdArc::Weight::One() );
  b.AddState();
  b.SetStart ( 0 );
  b.AddState();
  b.AddArc ( 0, fst::StdArc ( 3, 3, 0.5, 1 ) );
  b.SetFinal ( 1, fst::StdArc::Weight::One() );
  fst::ArcSort ( &a, fst::OLabelCompare<fst::StdArc>() );
  ///Don't add actual epsilons
  std::vector<fst::StdArc::Label> epsilons;
  epsilons.push_back ( DR );
  epsilons.push_back ( OOV );
  //Ready to use the multiepsilon compose
  //Compare to traditional approach:
  // Traverse a and relabel OOV and DR to epsilon
  std::vector<pair< fst::StdArc::Label, fst::StdArc::Label> > ipairs;
  std::vector<pair< fst::StdArc::Label, fst::StdArc::Label> > opairs;
  opairs.push_back ( pair< fst::StdArc::Label, fst::StdArc::Label> ( OOV,
                     EPSILON ) );
  opairs.push_back ( pair< fst::StdArc::Label, fst::StdArc::Label> ( DR,
                     EPSILON ) );
  //Both approaches should be equivalent, right?
  //test epsilon-free determinized lattices resulting from both methods...
  EXPECT_TRUE ( Equivalent ( fst::VectorFst<fst::StdArc>
                             ( fst::DeterminizeFst<fst::StdArc> ( fst::MultiEpsilonCompose<fst::StdArc> ( a,
                                 b, epsilons ) ) ),
                             fst::VectorFst<fst::StdArc> ( fst::DeterminizeFst<fst::StdArc>
                                 ( fst::ProjectFst<fst::StdArc> ( fst::ComposeFst<fst::StdArc>
                                     ( fst::RelabelFst<fst::StdArc> ( a, ipairs, opairs ), b ),
                                     fst::PROJECT_INPUT ) ) ) ) );
};

///Testing composition with failure transition phi.
TEST ( fstutils, phicompose ) {
  fst::VectorFst<fst::StdArc> a, b, c;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( DR, DR, 0.5, 1 ) );
  a.AddState();
  a.SetFinal ( 1, fst::StdArc::Weight::One() );
  b.AddState();
  b.SetStart ( 0 );
  b.AddState();
  b.AddArc ( 0, fst::StdArc ( PHI, PHI, 0, 1 ) );
  b.AddState();
  b.AddArc ( 1, fst::StdArc ( DR, DR, 0, 2 ) );
  b.SetFinal ( 2, fst::StdArc::Weight::One() );
  fst::ArcSort ( &a, fst::OLabelCompare<fst::StdArc>() );
  EXPECT_TRUE ( Equivalent ( RPhiCompose ( a, b, PHI ), a ) );
  //Testing special phi-transition mode: composition with a phi as a cyclic label in a state
  c.AddState();
  c.SetStart ( 0 );
  c.AddState();
  c.AddArc ( 0, fst::StdArc ( DR, DR, 0, 1 ) );
  c.AddArc ( 1, fst::StdArc ( PHI, PHI, 0, 1 ) );
  c.SetFinal ( 1, fst::StdArc::Weight::One() );
  EXPECT_TRUE ( Equivalent ( fst::RPhiCompose ( a, c, PHI ), a ) );
};

///Trivial testing simple language model application with kenlm
TEST ( fstutils, applylmonthefly ) {
  {
    ucam::util::oszfstream o ( "mylm" );
    o << endl;
    o << "\\data\\" << endl;
    o << "ngram 1=4" << endl;
    o << "ngram 2=2" << endl;
    o << "ngram 3=1" << endl;
    o << endl;
    o << "\\1-grams:" << endl;
    o << "-1\t3\t0" << endl;
    o << "-10\t4\t0" << endl;
    o << "-100\t</s>\t0" << endl;
    o << "0\t<s>\t0" << endl;
    o << endl;
    o << "\\2-grams:" << endl;
    o << "-1000\t3 4\t0" << endl;
    o << "-10000\t4 </s>\t0" << endl;
    o << endl;
    o << "\\3-grams:" << endl;
    o << "-100000\t3 4 </s>" << endl;
    o << endl;
    o << "\\end\\" << endl;
    o.close();
  }
  //Build here the resulting lattice with the expected value
  fst::VectorFst<fst::StdArc> a;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( 1, 1, 0, 1 ) );
  a.AddState();
  a.AddArc ( 1, fst::StdArc ( 3, 3, 1, 2 ) );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 4, 4, 1000, 3 ) );
  a.AddState();
  a.AddArc ( 3, fst::StdArc ( 2, 2, 100000, 4 ) );
  a.AddState();
  a.SetFinal ( 4, fst::StdArc::Weight::One() );
  fst::VectorFst<fst::StdArc> c ( a );
  //Delete scores, apply lm on-the-fly and see if it matches!
  fst::Map<fst::StdArc> ( &c, fst::RmWeightMapper<fst::StdArc>() );
  std::tr1::unordered_set<fst::StdArc::Label> epsilons;
  lm::ngram::Config kenlm_config;
  ucam::fsttools::IdBridge idb;
  lm::HifstEnumerateVocab hev (idb, NULL);
  kenlm_config.enumerate_vocab = &hev;
  fst::MakeWeight<fst::StdArc> mw;
  lm::ngram::Model *model = new lm::ngram::Model ( "mylm" , kenlm_config);
  fst::ApplyLanguageModelOnTheFly<fst::StdArc> *f = new
    fst::ApplyLanguageModelOnTheFly<fst::StdArc> (*model, epsilons, false, 1 ,0 , idb, mw);
  
  fst::VectorFst<fst::StdArc> *output = f->run(c);
  EXPECT_TRUE ( Equivalent ( *output, a ) );
  delete model;
  delete f;
  delete output;
  bfs::remove ( bfs::path ( "mylm" ) );
};

namespace googletesting {
//Just for test purposes, a functor that would simply delete weights.
struct RemoveWeight {
  const fst::StdArc::Weight operator() ( const fst::StdArc::Weight& w ) const {
    return w.Value() != fst::StdArc::Weight::Zero() ? fst::StdArc::Weight::One() :
           w;
  };
};

}
///Test the generic weight automapper, with a trivial modification of the weights, i.e. remove weight
TEST ( fstutils, genericweightautomapper ) {
  fst::VectorFst<fst::StdArc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::StdArc::Weight::One() );
  a.AddArc ( 0, fst::StdArc ( 10, 10, 0.5, 1 ) );
  fst::VectorFst<fst::StdArc> b ( a );
  fst::Map<fst::StdArc> ( &b, fst::RmWeightMapper<fst::StdArc>() );
  googletesting::RemoveWeight rw;
  fst::Map<fst::StdArc> ( &a,
                          fst::GenericWeightAutoMapper<fst::StdArc, googletesting::RemoveWeight> ( rw ) );
  EXPECT_TRUE ( Equivalent ( a, b ) );
}

///Test the generic weight mapper, with a trivial modification of the weights, i.e. remove weight
TEST ( fstutils, genericweightmapper ) {
  fst::VectorFst<fst::StdArc> a;
  a.AddState();
  a.AddState();
  a.SetStart ( 0 );
  a.SetFinal ( 1, fst::StdArc::Weight::One() );
  a.AddArc ( 0, fst::StdArc ( 10, 10, 0.5, 1 ) );
  fst::VectorFst<fst::StdArc> b ( a );
  fst::Map<fst::StdArc> ( &b, fst::RmWeightMapper<fst::StdArc>() );
  fst::VectorFst<fst::StdArc> c;
  googletesting::RemoveWeight rw;
  fst::Map ( a, &c,
             fst::GenericWeightMapper<fst::StdArc, fst::StdArc, googletesting::RemoveWeight>
             ( rw ) );
  EXPECT_TRUE ( Equivalent ( c, b ) );
}

TEST ( fstutils, multiunionrational ) {
  fst::VectorFst<fst::StdArc> *a = new fst::VectorFst<fst::StdArc>;
  a->AddState();
  a->AddState();
  a->SetStart ( 0 );
  a->SetFinal ( 1, fst::StdArc::Weight::One() );
  a->AddArc ( 0, fst::StdArc ( 10, 10, 0.5, 1 ) );
  fst::VectorFst<fst::StdArc> * b = new fst::VectorFst<fst::StdArc>;
  b->AddState();
  b->AddState();
  b->SetStart ( 0 );
  b->SetFinal ( 1, fst::StdArc::Weight::One() );
  b->AddArc ( 0, fst::StdArc ( 100, 100, 0.1, 1 ) );
  fst::VectorFst<fst::StdArc> * c = new fst::VectorFst<fst::StdArc>;
  c->AddState();
  c->AddState();
  c->SetStart ( 0 );
  c->SetFinal ( 1, fst::StdArc::Weight::One() );
  c->AddArc ( 0, fst::StdArc ( 1000, 1000, 0.1, 1 ) );
  fst::VectorFst<fst::StdArc> * d = new fst::VectorFst<fst::StdArc>;
  d->AddState();
  d->AddState();
  d->SetStart ( 0 );
  d->SetFinal ( 1, fst::StdArc::Weight::One() );
  d->AddArc ( 0, fst::StdArc ( 10000, 10000, 0.1, 1 ) );
  fst::MultiUnionRational<fst::StdArc> m;
  m.Add ( a );
  m.Add ( b );
  m.Add ( c );
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > j ( m() );
  RmEpsilon ( j.get() );
  ///Now add another fst, and do the union
  m.Add ( d );
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > j2 ( m() );
  RmEpsilon ( j2.get() );
  /// Create union of full fsts
  Union ( a, *b );
  Union ( a, *c );
  RmEpsilon ( a );
  EXPECT_TRUE ( Equivalent ( *j, *a ) );
  Union ( a, *d );
  RmEpsilon ( a );
  EXPECT_TRUE ( Equivalent ( *j2, *a ) );
  ///Note that fst in a has been modified (union). Therefore the result of the unioon is modified too.
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > j3 ( m() );
  ///Not deterministic, cannot use Equivalent directly, but the following check will suffice
  RmEpsilon ( j3.get() );
  EXPECT_TRUE ( j3.get()->NumStates() != a->NumStates() );
}

TEST ( fstutils, multiunionreplace ) {
  ///Exactly the same test
  fst::VectorFst<fst::StdArc> *a = new fst::VectorFst<fst::StdArc>;
  a->AddState();
  a->AddState();
  a->SetStart ( 0 );
  a->SetFinal ( 1, fst::StdArc::Weight::One() );
  a->AddArc ( 0, fst::StdArc ( 10, 10, 0.5, 1 ) );
  fst::VectorFst<fst::StdArc> * b = new fst::VectorFst<fst::StdArc>;
  b->AddState();
  b->AddState();
  b->SetStart ( 0 );
  b->SetFinal ( 1, fst::StdArc::Weight::One() );
  b->AddArc ( 0, fst::StdArc ( 100, 100, 0.1, 1 ) );
  fst::VectorFst<fst::StdArc> * c = new fst::VectorFst<fst::StdArc>;
  c->AddState();
  c->AddState();
  c->SetStart ( 0 );
  c->SetFinal ( 1, fst::StdArc::Weight::One() );
  c->AddArc ( 0, fst::StdArc ( 1000, 1000, 0.1, 1 ) );
  fst::VectorFst<fst::StdArc> * d = new fst::VectorFst<fst::StdArc>;
  d->AddState();
  d->AddState();
  d->SetStart ( 0 );
  d->SetFinal ( 1, fst::StdArc::Weight::One() );
  d->AddArc ( 0, fst::StdArc ( 10000, 10000, 0.1, 1 ) );
  fst::MultiUnionRational<fst::StdArc> m;
  m.Add ( a );
  m.Add ( b );
  m.Add ( c );
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > j ( m() );
  RmEpsilon ( j.get() );
  ///Now add another fst, and do the union
  m.Add ( d );
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > j2 ( m() );
  RmEpsilon ( j2.get() );
  /// Create union of full fsts
  Union ( a, *b );
  Union ( a, *c );
  RmEpsilon ( a );
  EXPECT_TRUE ( Equivalent ( *j, *a ) );
  Union ( a, *d );
  RmEpsilon ( a );
  EXPECT_TRUE ( Equivalent ( *j2, *a ) );
  ///Note that fst in a has been modified (union). Therefore the result of the unioon is modified too.
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > j3 ( m() );
  ///Not deterministic, cannot use Equivalent directly, but the following check will suffice
  RmEpsilon ( j3.get() );
  EXPECT_TRUE ( j3.get()->NumStates() != a->NumStates() );
}

TEST (fstutils, extractngrams ) {
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
  std::vector<fst::NGram> ng;
  fst::extractNGrams<fst::StdArc> (a, ng, 5);
  std::stringstream ss;
  for (uint k = 0; k < ng.size(); ++k)
    ss << ng[k] << endl;
  std::string ngrams =
    "1\n1 2\n1 2 3\n1 2 3 4\n1 2 6\n1 2 6 7\n1 5\n1 5 4\n2\n2 3\n2 3 4\n2 6\n2 6 7\n3\n3 4\n4\n5\n5 4\n6\n6 7\n7\n";
  EXPECT_TRUE (ngrams == ss.str() );
  fst::extractNGrams<fst::StdArc> (b, ng, 5);
}

TEST ( fstutils, string2fst) {
  fst::VectorFst<fst::StdArc> a, b;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( 1, 1, 0, 1 ) );
  a.AddState();
  a.AddArc ( 1, fst::StdArc ( 3, 3, 1, 2 ) );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 4, 4, 1000, 3 ) );
  a.AddState();
  a.AddArc ( 3, fst::StdArc ( 2, 2, 100000, 4 ) );
  a.SetFinal ( 4, fst::StdArc::Weight::One() );
  fst::string2fst<fst::StdArc> ("1 3 4 2", &b, "", fst::StdArc::Weight (101001) );
  EXPECT_EQ (Equivalent (a, b), true);
  b.DeleteStates();
  fst::string2fst<fst::StdArc> ("1 3 4 2", &b, "1 3 4 2",
                                fst::StdArc::Weight (101001) );
  EXPECT_EQ (Equivalent (a, b), true);
}

//Test relabeutil functor -- just a wrapper for relabeling fsts
TEST ( fstutils, relabelutil) {
  fst::VectorFst<fst::StdArc> a;
  a.AddState();
  a.SetStart ( 0 );
  a.AddState();
  a.AddArc ( 0, fst::StdArc ( 1, 1, 0, 1 ) );
  a.AddState();
  a.AddArc ( 1, fst::StdArc ( 3, 3, 1, 2 ) );
  a.AddState();
  a.AddArc ( 2, fst::StdArc ( 4, 4, 1000, 3 ) );
  a.AddState();
  a.AddArc ( 3, fst::StdArc ( 2, 2, 100000, 4 ) );
  a.SetFinal ( 4, fst::StdArc::Weight::One() );
  fst::RelabelUtil<fst::StdArc> rb;
  fst::VectorFst<fst::StdArc> b (a);
  //Empty should be identical to a, of course...
  EXPECT_EQ (Equivalent (rb (b), a),
             true);
  ///Now lets replace a couple of symbols in the openfst way --
  std::vector<pair <fst::StdArc::Label, fst::StdArc::Label> > ipairs;
  std::vector<pair <fst::StdArc::Label, fst::StdArc::Label> >  opairs;
  ipairs.push_back (pair <fst::StdArc::Label, fst::StdArc::Label> (3, 3000 ) );
  ipairs.push_back (pair <fst::StdArc::Label, fst::StdArc::Label> (4, 4000 ) );
  opairs.push_back (pair <fst::StdArc::Label, fst::StdArc::Label> (3, 3000) );
  opairs.push_back (pair <fst::StdArc::Label, fst::StdArc::Label> (4, 4000) );
  Relabel (&a, ipairs, opairs);
  //and test...
  EXPECT_EQ (Equivalent (rb.addIPL (3, 3000).addOPL (3, 3000).addIPL (4,
                         4000).addOPL (4,
                                       4000) (b),
                         a),
             true);
}

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
