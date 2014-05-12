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

#ifndef LMERT_H_
#define LMERT_H_

#include <vector>

#include <fst/fstlib.h>

#include "MertCommon.h"
#include "function-weight.h"

bool GradientSortPredicate (const MertLine& line1, const MertLine& line2);

class MertEnvelope {
 public:
  void SortLines(); // sort envelope lines by slope
  void SweepLine(); // compute upper envelope of lines in array
  std::string ToString (
    bool); // returns lines that constitute the envelope as a string
  std::vector<MertLine> lines; // lines that define the envelope / convex hull
};

class MertLattice {
 public:
  MertLattice (TupleArcFst *, const PARAMS&, const PARAMS&);
  void InitializeEnvelopes();
  void InitializeStartState();
  void PropagateEnvelope (const TupleArcFst::StateId&,
                          const TupleArcFst::StateId&,
                          const TupleW&, const Wid& w);
  void ComputeStateEnvelopes();
  void ComputeFinalEnvelope();
  MertEnvelope ComputeLatticeEnvelope();
 private:
  TupleArcFst* fst;
  std::vector<MertEnvelope> envelopes;
  PARAMS lambda;
  PARAMS direction;
};

class LMertAlgorithm {
 public:
  typedef std::vector<MertLine> Lines;

  static Lines const ComputeLatticeEnvelope (TupleArcFst*, const PARAMS&,
      const PARAMS&);
};

#endif /* LMERT_H_ */
