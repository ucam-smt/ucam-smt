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
 * \brief Unit testing: Input/Output fst operations with compression.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <openfst.h>
#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "szfstream.hpp"
#include "fstio.hpp"

namespace bfs = boost::filesystem;

namespace googletesting {

///Basic test for input/output enhanced fst operations
///Create an vector/const fst, write to compressed binary format, read back and test that both fsts are equivalent.
TEST ( FstIo, basic_test ) {
  fst::VectorFst<fst::StdArc> aux;
  aux.AddState();
  aux.AddState();
  aux.SetStart ( 0 );
  aux.SetFinal ( 1, fst::StdArc::Weight::One() );
  aux.AddArc ( 0, fst::StdArc ( 10, 10, 0.5, 1 ) );
  FstWrite ( aux, "obliviate.fst.gz" );
  boost::scoped_ptr< fst::VectorFst<fst::StdArc> > aux2 (
    fst::VectorFstRead<fst::StdArc> ( "obliviate.fst.gz" ) );
  EXPECT_TRUE ( Equivalent ( aux, *aux2 ) );
  bfs::remove ( bfs::path ( "obliviate.fst.gz" ) );
  fst::ConstFst<fst::StdArc> caux ( aux );
  FstWrite ( caux, "const.obliviate.fst.gz" );
  FstWrite ( caux, "const.obliviate.txt" );
  /*
   * \todo This test is no longer working in openfst 1.3.4 but works in openfst.1.3.2. Needs investigation.
  FstWrite ( caux, "const.obliviate.fst" );
  iszfstream file ( "const.obliviate.fst" );
  FstReadOptions fro;
  ConstFst<StdArc> *h = ConstFst<StdArc>::Read ( *file.getStream(), fro );
  if (h == NULL) {
    LERROR("Ufa?");
    exit(0);
  }

  boost::scoped_ptr< fst::ConstFst<fst::StdArc> > caux2( fst::ConstFstRead<fst::StdArc> ( "const.obliviate.fst.gz" ) ) ;
  EXPECT_TRUE ( Equivalent ( caux, *caux2 ) );
  */
  bfs::remove ( bfs::path ( "const.obliviate.fst.gz" ) );

  ucam::util::iszfstream isz ( "const.obliviate.txt" );
  std::string t, t2;
  while ( getline ( isz, t ) ) t2 += t + "\n";
  EXPECT_EQ ( t2, "0\t1\t10\t10\t0.5\n1\n" );
  bfs::remove ( bfs::path ( "const.obliviate.txt" ) );
}

};

#ifndef GMAINTEST

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
