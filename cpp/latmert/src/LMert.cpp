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

#include <LMert.h>

std::vector<MertLine> const LMertAlgorithm::ComputeLatticeEnvelope (
  TupleArcFst* vec, const PARAMS& lambda,
  const PARAMS& direction) {
  MertLattice lattice (vec, lambda, direction);
  return lattice.ComputeLatticeEnvelope().lines;
}

bool GradientSortPredicate (const MertLine& line1, const MertLine& line2) {
  return line1.m < line2.m;
}

void MertEnvelope::SortLines() {
  sort (lines.begin(), lines.end(), GradientSortPredicate);
}

void MertEnvelope::SweepLine() {
  SortLines();
  int j = 0;
  for (std::vector<MertLine>::size_type i = 0; i < lines.size(); i++) {
    MertLine l = lines[i];
    l.x = -std::numeric_limits<double>::infinity();
    if (0 < j) {
      if (lines[j - 1].m == l.m) {
        if (l.y <= lines[j - 1].y)
          continue;
        --j;
      }
      while (0 < j) {
        l.x = (l.y - lines[j - 1].y) / (lines[j - 1].m - l.m);
        if (lines[j - 1].x < l.x)
          break;
        --j;
      }
      if (0 == j)
        l.x = -std::numeric_limits<double>::infinity();
      lines[j++] = l;
    } else {
      lines[j++] = l;
    }
  }
  lines.resize (j);
}

std::string MertEnvelope::ToString (bool show_hypothesis = false) {
  std::ostringstream oss;
  for (std::vector<MertLine>::size_type i = 0; i < lines.size(); ++i) {
    oss << "line i=[" << std::right << std::setw (4) << i << "]" << std::fixed
        << std::setprecision (6) << " x=[" << std::right << std::setw (12) << lines[i].x
        << "]" << " y=[" << std::right << std::setw (12) << lines[i].y << "]"
        << " m=[" << std::right << std::setw (12) << lines[i].m << "]";
    if (show_hypothesis) {
      oss << " t=[" << lines[i].t << "]";
    }
    oss << '\n';
  }
  return oss.str();
}

MertLattice::MertLattice (TupleArcFst* fst, const PARAMS& lambda,
                          const PARAMS& direction) :
  fst (fst), lambda (lambda), direction (direction) {
}

void MertLattice::InitializeEnvelopes() {
  envelopes.resize (fst->NumStates() + 1);
}

void MertLattice::InitializeStartState() {
  envelopes[fst->Start()].lines.push_back (MertLine() );
}

void MertLattice::PropagateEnvelope (const TupleArcFst::StateId& src,
                                     const TupleArcFst::StateId& trg, const TupleW& features, const Wid& w = 0) {
  for (unsigned int i = 0; i < envelopes[src].lines.size(); ++i) {
    MertLine line (envelopes[src].lines[i]);
    line.y += DotProduct (features, lambda) * -1;
    line.m += DotProduct (features, direction) * -1;
    if (w != 0) {
      line.t.push_back (w);
    }
    envelopes[trg].lines.push_back (line);
  }
}

void MertLattice::ComputeStateEnvelopes() {
  for (fst::StateIterator < TupleArcFst > si (*fst); !si.Done(); si.Next() ) {
    const TupleArc::StateId& s = si.Value();
    envelopes[s].SweepLine();
    for (fst::ArcIterator < TupleArcFst > ai (*fst, si.Value() ); !ai.Done();
         ai.Next() ) {
      const TupleArc& a = ai.Value();
      PropagateEnvelope (s, a.nextstate, a.weight, a.ilabel);
    }
    if (fst->Final (s) != TupleW::Zero() ) {
      PropagateEnvelope (s, fst->NumStates(), fst->Final (s) );
    }
    envelopes[s].lines.clear();
  }
}

void MertLattice::ComputeFinalEnvelope() {
  envelopes[fst->NumStates()].SweepLine();
}

MertEnvelope MertLattice::ComputeLatticeEnvelope() {
  InitializeEnvelopes();
  InitializeStartState();
  ComputeStateEnvelopes();
  ComputeFinalEnvelope();
  return envelopes[fst->NumStates()];
}
