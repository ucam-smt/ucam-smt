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

#include "DebugMert.h"

MertList const DebugMertAlgorithm::ComputeLatticeEnvelope (TupleArcFst* vec,
    const PARAMS& lambda,
    const PARAMS& direction) {
  MertList tgList = TGMertAlgorithm<TupleArc>::ComputeLatticeEnvelope (vec,
                    lambda,
                    direction);
  std::vector<MertLine> lList = LMertAlgorithm::ComputeLatticeEnvelope (vec,
                                lambda, direction);
  //We assume that both TGMERT and LMERT compute identical interval boundaries
  bool identical = true;
  std::vector<MertLine>::iterator lit = lList.begin();
  for (MertIter tgit = tgList.begin(); tgit != tgList.end(); ++tgit) {
    identical &= tgit->t == lit->t;
    ++lit;
  }
  if ( ! identical) {
    cout << "TGMERT: " << endl;
    for (MertIter tgit = tgList.begin(); tgit != tgList.end(); ++tgit) {
      cout << tgit->x << "\t"  << tgit->t << endl;
    }
    cout << "LMERT: " << endl;
    for (std::vector<MertLine>::iterator lit = lList.begin(); lit != lList.end();
         ++lit) {
      cout << lit->x << "\t"  << lit->t << endl;
    }
  }
  return tgList;
}
