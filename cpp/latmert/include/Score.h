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

#ifndef SCORE_H_
#define SCORE_H_

#include <BleuStats.h>
#include "TuneSet.h"
#include "function-weight.h"

Sentence GetBestHypothesis (const TupleArcFst*, const PARAMS&);

template<typename RefData>
typename RefData::ErrorStats::Error ComputeError (RefData& refData,
    const TuneSet& lats, const PARAMS& lambda) {
  typename RefData::ErrorStats aggregate;
  for (std::vector<Sid>::const_iterator sit = lats.ids.begin();
       sit != lats.ids.end(); ++sit) {
    if (opts.verbose) {
      tracer << "sentence s=" << *sit << '\n';
    }
    TupleArcFst* fst = lats.GetVectorLattice (*sit, opts.useCache);
    if (!fst) {
      cerr << "ERROR: invalid vector lattice for sentence s=" << *sit
           << '\n';
      exit (1);
    }
    Sentence h = GetBestHypothesis (fst, lambda);
    typename RefData::ErrorStats es = refData.ComputeErrorStats (*sit, h);
    if (opts.verbose) {
      tracer << "best hypothesis=" << h << " " << es << '\n';
    }
    aggregate = aggregate + es;
    if (!opts.useCache) {
      delete fst;
    }
  }
  return aggregate.ComputeError();
}

template<typename RefData>
std::string Score (RefData& refData, const TuneSet& lats,
                   const PARAMS& lambda) {
  typename RefData::ErrorStats::Error error = ComputeError<RefData> (refData,
      lats, lambda);
  std::stringstream ss;
  ss << error;
  return ss.str();
}

class Scorer {
 public:
  virtual std::string operator() (const std::vector<std::string>& refFilenames,
                                  const TuneSet& lats,
                                  const PARAMS& lambda) = 0;
  virtual ~Scorer() {
  }
  ;
};

template<typename RefData>
class ScorerImpl: public Scorer {

  virtual std::string operator() (const std::vector<std::string>& refFilenames,
                                  const TuneSet& lats,
                                  const PARAMS& lambda) {
    RefData refData;
    refData.LoadRefData (refFilenames);
    return Score (refData, lats, lambda);
  }
};

#endif /* SCORE_H_ */
