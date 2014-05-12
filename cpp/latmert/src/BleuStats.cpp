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

#include "BleuStats.h"
#include <cmath>
#include <cstddef>
using std::min;
using std::log;

const unsigned int BleuStats::MAX_BLEU_ORDER = 4;

BleuStats::BleuStats() : refLength (0) {
  tots.resize (MAX_BLEU_ORDER, 0);
  hits.resize (MAX_BLEU_ORDER, 0);
}

BleuStats::Error BleuStats::ComputeError (const unsigned int order) const {
  double logBleu = 0.0;
  double logBrev = 0.0;
  for (unsigned int n = 0; n < order; ++n) {
    logBleu += log ( (double) this->hits[n] / (double) this->tots[n]);
  }
  logBleu *= 1 / (double) BleuStats::MAX_BLEU_ORDER;
  logBrev = min (0.0, 1 - this->refLength / (double) (this->tots[0]) );
  return Error (exp (logBleu + logBrev), exp (logBrev) );
}

ostream& operator<< (ostream& o, const BleuStats& b) {
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    if (n > 0) {
      o << "\t";
    }
    o << b.hits[n] << "\t" << b.tots[n];
  }
  o << "\t" << b.refLength;
  return o;
}

BleuStats operator+ (const BleuStats& bs1, const BleuStats& bs2) {
  BleuStats bs;
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    bs.hits[n] = bs1.hits[n] + bs2.hits[n];
    bs.tots[n] = bs1.tots[n] + bs2.tots[n];
  }
  bs.refLength = bs1.refLength + bs2.refLength;
  return bs;
}

BleuStats operator- (const BleuStats& bs1, const BleuStats& bs2) {
  BleuStats bs;
  bs.refLength = bs1.refLength - bs2.refLength;
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    bs.tots[n] = bs1.tots[n] - bs2.tots[n];
    bs.hits[n] = bs1.hits[n] - bs2.hits[n];
  }
  return bs;
}

ostream& operator<< (ostream& o, const Bleu& b) {
  o << b.m_bleu << " " << b.m_brev;
  return o;
}

bool operator> (Bleu& b1, Bleu& b2) {
  return b1.m_bleu > b2.m_bleu;
}

