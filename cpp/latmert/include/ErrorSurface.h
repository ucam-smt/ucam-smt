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

#ifndef ERRORSURFACE_H_
#define ERRORSURFACE_H_

#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>
#include <sstream>
#include <boost/thread/mutex.hpp>

#include "MertCommon.h"
#include "BleuStats.h"
#include "IntervalData.h"

template<typename IntervalBoundary>
bool IntervalBoundarySortPredicate (const IntervalBoundary& b1,
                                    const IntervalBoundary& b2) {
  return b1.gamma < b2.gamma;
}

template<typename RD>
class ErrorSurface {
 public:
  typedef RD RefData;
  typedef typename RefData::ErrorStats ErrorStats;
  typedef IntervalData<ErrorStats> IntervalBoundary;
  typedef typename ErrorStats::Error Error;

 private:

  ErrorStats MergeInitials() const {
    ErrorStats es;
    for (typename std::vector<IntervalBoundary>::const_iterator it =
           ++ (initials.begin() ); it != initials.end(); ++it) {
      es = es + it->errorStats;
    }
    return es;
  }

  /*
   * Slightly hacky way of updating both the translation and expected translation score. First element of
   * referenced array is the translation score and the second is the expected score.
   */
  void MergeInitialScores (double scores[]) const {
    scores[0] = 0.0;
    scores[1] = 0.0;
    for (typename std::vector<IntervalBoundary>::const_iterator it =
           ++ (initials.begin() ); it != initials.end(); ++it) {
      scores[0] += it->deltaScore;
      scores[1] += it->deltaExpScore;
    }
  }

  double optimalGamma;
  Error optimalError;
  std::vector<ErrorStats> prev;
  RefData* refs;
  bool unbounded;
  std::vector<IntervalBoundary>
  initials; // Interval boundaries for left-most line segments (gamma = -infinity)
  std::vector<IntervalBoundary> prevBestIBs;
  boost::mutex mutex;

 public:

  ErrorSurface (unsigned int noOfSentences, RefData* refs) :
    refs (refs) {
    initials.resize (noOfSentences + 1);
    prevBestIBs.resize (noOfSentences + 1);
    prev.resize (noOfSentences + 1);
  }

  ErrorSurface (const ErrorSurface& other) :
    optimalGamma (other.optimalGamma), optimalError (other.optimalError), prev (
      other.prev), refs (other.refs), unbounded (other.unbounded), initials (
        other.initials), prevBestIBs (other.prevBestIBs) {
  }

  ErrorSurface& operator= (const ErrorSurface& rhs) {
    if (this == &rhs) {
      return *this;
    }
    optimalGamma = rhs.optimalGamma;
    optimalError = rhs.optimalError;
    prev = rhs.prev;
    refs = rhs.refs;
    unbounded = rhs.unbounded;
    initials = rhs.initials;
    prevBestIBs = rhs.prevBestIBs;
    return *this;
  }

  double GetOptimalGamma() {
    return optimalGamma;
  }

  Error GetOptimalError() {
    return optimalError;
  }

  bool GetUnbounded() {
    return unbounded;
  }

  void ComputeSurface() {
    std::vector<IntervalBoundary> currentIBs (initials);
    double scores[2];
    MergeInitialScores (scores);
    ErrorStats aggregateErrorStats = MergeInitials();
    sort (boundaries.begin(), boundaries.end(),
          IntervalBoundarySortPredicate<IntervalBoundary>);
    optimalGamma = boundaries.front().gamma - 1;
    optimalError = aggregateErrorStats.ComputeError();
    typename std::vector<IntervalBoundary>::iterator itNext =
      ++ (boundaries.begin() );
    unbounded = true;
    for (typename std::vector<IntervalBoundary>::iterator it =
           boundaries.begin(); it != boundaries.end(); ++it) {
      scores[0] = it->score + it->deltaScore;
      scores[1] = it->expScore + it->deltaExpScore;
      it->score = scores[0];
      it->expScore = scores[1];
      aggregateErrorStats = aggregateErrorStats + it->errorStats;
      currentIBs[it->sentence].errorStats =
        currentIBs[it->sentence].errorStats + it->errorStats;
      Error current = aggregateErrorStats.ComputeError();
      if (current > optimalError) {
        copy (++ (currentIBs.begin() ), currentIBs.end(),
              ++ (prevBestIBs.begin() ) );
        optimalError = current;
        if (itNext == boundaries.end() ) {
          optimalGamma = it->gamma + 1;
          unbounded = true;
        } else {
          optimalGamma = 0.5 * (it->gamma + itNext->gamma);
          unbounded = false;
        }
      }
      ++itNext;
    }
  }

  void Reset() {
    boundaries.clear();
    optimalGamma = 0.0;
    Error zero;
    optimalError = zero;
    copy (++ (prevBestIBs.begin() ), prevBestIBs.end(), ++ (initials.begin() ) );
  }

  void PrintErrorSurface (ostream& os) const {
    ErrorStats aggregate;
    os << optimalGamma << " " << optimalError << std::endl;
    for (typename std::vector<IntervalBoundary>::const_iterator it =
           initials.begin(); it != initials.end(); ++it) {
      aggregate = aggregate + it->errorStats;
      os << *it << std::endl;
    }
    IntervalBoundary temp;
    for (typename std::vector<IntervalBoundary>::const_iterator it =
           boundaries.begin(); it != boundaries.end(); ++it) {
      temp = *it;
      aggregate = aggregate + it->errorStats;
      temp.errorStats = aggregate;
      os << temp << std::endl;
    }
  }

  void WriteErrorSurface (const std::string& filename) const {
    static unsigned int lineOptCount = 0;
    std::stringstream sstream;
    sstream << filename << lineOptCount;
    std::ofstream ofs (sstream.str().c_str() );
    if (!ofs.good() ) {
      std::cerr << "ERROR: can't write: " << sstream.str() << '\n';
      exit (1);
    }
    tracer << "writing error surface to " << sstream.str() << '\n';
    PrintErrorSurface (ofs);
    ofs.close();
    ++lineOptCount;
  }

  void CreateInitial (Sid sid, const double gamma, const Sentence h,
                      const double modelScore, const double expScore) {
    prev[sid] = refs->ComputeErrorStats (sid, h);
    initials[sid] = IntervalBoundary (sid, gamma, prev[sid], modelScore,
                                      expScore);
  }

  void CreateInterval (Sid sid, const double gamma, const Sentence h,
                       const double modelScore, const double expScore) {
    ErrorStats next = refs->ComputeErrorStats (sid, h);
    IntervalBoundary b (sid, gamma, next - prev[sid], modelScore, expScore);
    prev[sid]  = next;
    mutex.lock();
    boundaries.push_back (b);
    mutex.unlock();
  }

  std::vector<IntervalBoundary>
  boundaries; // Interval boundaries where top-most line segment changes

};

#endif /* ERRORSURFACE_H_ */
