//Copyright (c) 2012, University of Cambridge
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met://
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of Cambridge nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <iostream>

#include <fst/fstlib.h>
#include <fst/flags.h>

#include "tropical-sparse-tuple-weight.h"
#include "tropical-sparse-tuple-weight-decls.h"

DEFINE_bool (expand,   false,
             "convert a 32 bit tuple arc to a 64 bit tuple arc");

DEFINE_bool (stdarc,   false,
             "map 32 bit vector weights to a single stdarc dimension");

DEFINE_bool (tuplearc,   false,
             "map a single 32 bit stdarc dimension to a sparse tuple arc");

DEFINE_int32 (k,       0,    "vector dimension to map");

DEFINE_bool (dot,   false, "perform a dot product on the vector weights");

int main (int argc, char **argv) {
  std::string usage = "vecmap\n\n  Usage:";
  usage += argv[0];
  InitFst (usage.c_str(), &argc, &argv, true);
  if (argc != 1) {
    std::cerr << "ERROR: invalid arguments" << std::endl;
    return 1;
  }
  TupleArcFst32* fst;
  if (FLAGS_tuplearc) {
    fst = new TupleArcFst32;
    fst::StdVectorFst* stdfst = fst::StdVectorFst::Read (std::cin,
                                fst::FstReadOptions() );
    fst::StdToVector<float> m (FLAGS_k);
    fst::Map (*stdfst, fst, StdToSparseMapper (m) );
    delete stdfst;
  } else {
    fst = TupleArcFst32::Read (std::cin, fst::FstReadOptions() );
  }
  if (FLAGS_stdarc || FLAGS_dot) {
    fst::StdVectorFst stdfst;
    if (FLAGS_stdarc) {
      fst::VectorToStd<float> m (FLAGS_k);
      fst::Map (*fst, &stdfst, SparseToStdMapper (m) );
    } else {
      fst::DotProductMap<float> m (fst::TropicalSparseTupleWeight<float>::Params() );
      fst::Map (*fst, &stdfst,
                fst::GeneralMapper<TupleArc32, fst::ArcTpl<FeatureWeight32>, fst::DotProductMap<float> >
                (m) );
    }
    stdfst.Write (std::cout, fst::FstWriteOptions() );
  } else if (FLAGS_expand) {
    TupleArcFst doublefst;
    fst::Expand m;
    fst::Map (*fst, &doublefst, fst::ExpandMapper (m) );
    doublefst.Write (std::cout, fst::FstWriteOptions() );
  } else {
    fst->Write (std::cout, fst::FstWriteOptions() );
  }
  delete fst;
  return 0;
}
