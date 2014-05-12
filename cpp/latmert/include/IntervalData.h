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


#ifndef GENERICINTERVALBOUNDARY_H_
#define GENERICINTERVALBOUNDARY_H_

#include "BleuStats.h"
#include "MertCommon.h"

template<typename ErrorStats>
class IntervalData {
public:
	typedef typename ErrorStats::Error Error;

	IntervalData() {};

	IntervalData(Sid sentence,  const double gamma, ErrorStats errorStats,
			const double deltaScore = 0.0, const double deltaExpScore = 0.0) :
			sentence(sentence), gamma(gamma), errorStats(errorStats), deltaScore(
					deltaScore), score(0.0), deltaExpScore(deltaExpScore), expScore(
					0.0) {
	}
	;
	Sid sentence;
	ErrorStats errorStats;
	double gamma;
	double deltaScore; // delta of translation score
	double score; // sum of translation score
	double deltaExpScore; // delta of expected translation score
	double expScore; // sum of expected translation score

	friend ostream& operator<<(ostream& os, const IntervalData<ErrorStats>& id){
		Error error = id.errorStats.ComputeError();
		os << id.sentence << " " << std::fixed << std::setprecision(8) << id.gamma << " "
		   << std::setprecision(4) << error << " " << std::setprecision(8)
		   << id.expScore << " " << id.score << '\t'
		   << id.errorStats;
		return os;
	}
};

template<typename IntervalBoundary>
bool IntervalBoundarySortPredicate(const IntervalBoundary& ib1,
		const IntervalBoundary& ib2);

#endif /* GENERICINTERVALBOUNDARY_H_ */
