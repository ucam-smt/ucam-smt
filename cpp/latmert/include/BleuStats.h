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


#ifndef BLEUSTATS_H_
#define BLEUSTATS_H_

#include <iostream>
#include <vector>
#include <limits>

using std::ostream;
using std::vector;

class Bleu {
public:
	Bleu(const double bleu = -std::numeric_limits<double>::infinity(),
			const double brev = 0) :
			m_bleu(bleu), m_brev(brev) {
	}
	inline double GetError() const {
		return m_bleu;
	}

	friend ostream& operator<<(ostream&, const Bleu&);

	friend bool operator>(Bleu&, Bleu&);

private:
	double m_bleu;
	double m_brev;
};



class BleuStats {
public:
	typedef Bleu Error;
	BleuStats();
	vector<int> tots; // order-specific ngram totals
	vector<int> hits; // order-specific ngram hits
	long refLength; // effective reference length
	Error ComputeError(const unsigned int=4) const;

	static const unsigned int MAX_BLEU_ORDER;
};

ostream& operator<<(ostream& o, const BleuStats& b);

BleuStats operator+(const BleuStats&, const BleuStats&);

BleuStats operator-(const BleuStats&, const BleuStats&);

#endif /* BLEUSTATS_H_ */
