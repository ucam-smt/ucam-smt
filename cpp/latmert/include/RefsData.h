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


#ifndef REFSDATA_H_
#define REFSDATA_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>
#include <tr1/unordered_map>

#include "BleuStats.h"
#include "MertHashVec.h"
#include "MertCommon.h"

using std::min;
using std::max;
using std::vector;
using std::string;
using std::ostream;
using std::ostringstream;
using std::numeric_limits;

typedef unordered_map<NGram, unsigned int, hashfvecint64,
		hasheqvecint64> NGramToCountMap;

class RefsData {
	RefsData();
	NGram SubStr(const Sentence& s, const unsigned int n,
			const unsigned int l) const;
	unsigned int ClosestReferenceLength(const unsigned int hypLength) const;
	NGramToCountMap refCounts; // Counts of ngrams in the reference translations
	vector<unsigned int> refLengths; // Lengths of the reference translations
public:
	typedef BleuStats ErrorStats;
	RefsData(const vector<Sentence>& refs);
	BleuStats ComputeBleuStats(const Sentence& hyp) const;
	string ToString(const char separator) const;
	static RefsData dummy;
};

typedef vector<RefsData> RefsDataIndex;

class IntegerEncRefs {
	RefsDataIndex refsData; // Map from sentence ID to reference ngrams

public:
	typedef BleuStats ErrorStats;
	void LoadRefData(vector<string>);
	ErrorStats ComputeErrorStats(Sid, Sentence) const;

};

ostream& operator<<(ostream& o, const Sentence& s);

#endif /* REFSDATA_H_ */
