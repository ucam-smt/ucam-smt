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

#include "TuneSet.h"
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/file.hpp>
#include <fstream>
#include <iostream>

DEFINE_string (lats, "", "path to vector lattices");
DEFINE_string (idxlimits, "", "sentence index limits 'min:max'");
DEFINE_string (idxscript, "", "script containing a list of sentence ids");

TuneSet::TuneSet() {
}

TuneSet::~TuneSet() {
  for (std::vector<TupleArcFst *>::iterator it = cachedLattices.begin();
       it != cachedLattices.end(); ++it) {
    delete *it;
  }
}

TupleArcFst* TuneSet::LoadLattice (const Sid s) const {
  boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
  in.push (boost::iostreams::gzip_decompressor() );
  in.push (boost::iostreams::file_source (ExpandPath (m_pattern, s) ) );
  std::istream is (&in);
  TupleArcFst32* fst32 = TupleArcFst32::Read (is, fst::FstReadOptions() );
  TopSort (fst32);
  if (!fst32) {
    cerr << "ERROR: unable to load vector lattice: " << ExpandPath (m_pattern,
         s) << '\n';
    exit (1);
  }
  if (fst32->Properties (fst::kNotTopSorted, true) ) {
    cerr << "ERROR: Input lattices are not topologically sorted: " << '\n';
    exit (1);
  }
  TupleArcFst* fst = new TupleArcFst;
  fst::Expand m;
  fst::Map (*fst32, fst, fst::ExpandMapper (m) );
  delete fst32;
  return fst;
}

void TuneSet::Initialize (const bool use_cache) {
  if (FLAGS_lats.empty() ) {
    cerr << "ERROR: mandatory parameter not specified: 'lats'\n";
    exit (1);
  }
  if (!FLAGS_idxlimits.empty() ) {
    InitializeFromLimits (ids, FLAGS_idxlimits.data() );
  }
  if (!FLAGS_idxscript.empty() ) {
    InitializeFromScript (ids, FLAGS_idxscript.data() );
  }
  if (ids.size() == 0) {
    cerr
        << "ERROR: must specify either 'idxlimits' or 'idxscript' parameters"
        << '\n';
    exit (1);
  }
  tracer << "idx min=" << ids.front() << '\n';
  tracer << "idx max=" << ids.back() << '\n';
  m_pattern = FLAGS_lats;
  if (use_cache) {
    cachedLattices.push_back (0);
    for (std::vector<Sid>::const_iterator sit = ids.begin(); sit != ids.end();
         ++sit) {
      cachedLattices.push_back (LoadLattice (*sit) );
    }
    tracer << ids.size() << " vector lattices loaded\n";
  }
}

TupleArcFst* TuneSet::GetVectorLattice (const Sid s,
                                        const bool use_cache) const {
  if (use_cache) {
    return cachedLattices[s];
  }
  return LoadLattice (s);
}
