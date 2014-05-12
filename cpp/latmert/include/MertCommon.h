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

#ifndef MERTCOMMON_H_
#define MERTCOMMON_H_

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "tropical-sparse-tuple-weight.h"
#include "tropical-sparse-tuple-weight-decls.h"


#define tracer std::cerr << Time()

struct MertOpt {
	int32 printPrecision;
	int32 latticeCutoff;
	double gammaThreshold;
	double bleuThreshold;
	bool verbose;
	std::string writeSurface;
	bool scaleParams;
	bool ignoreGsf;
	bool useCache;
	bool noSkip;
	bool fullLog;
	bool pointTest;
};

extern MertOpt opts;

typedef unsigned int Sid; // Input sentence id
typedef long long Wid; // Word id
typedef std::vector<Wid> NGram;
typedef std::vector<Wid> Sentence; // Hypothesis

std::string Time();

const double kDoubleDelta = 1.0F / 4096.0F;

void ReplacePattern(std::string& newname, std::string pattern, const char *pat,
		std::string rep);
void ReplacePattern(std::string& newname, std::string pattern, const char *pat, float f);
void ReplacePattern(std::string& newname, std::string pattern, const char *pat, int i);

std::string ExpandPath(std::string pattern, const int idx);

void InitializeFromLimits(std::vector<Sid>& ids, const std::string limits);
void InitializeFromScript(std::vector<Sid>& ids, const std::string filename);

//string EnvelopeToString(const MertList&, bool);

std::vector<PARAMS> InitializeVectorsFromFile(const std::string&);

std::vector<PARAMS> InitializeVectorsFromAxes(unsigned int);

std::vector<PARAMS> InitializeVectorsFromRandom();

std::vector<PARAMS> InitializeVectors(const std::string&);

std::string ReadWeight(const std::string&);

std::ostream& operator<<(std::ostream &strm, const PARAMS &);

std::ostream& operator<<(std::ostream& o, const Sentence& s);

PARAMS operator-(const PARAMS&, const PARAMS&);

PARAMS operator+(const PARAMS&, const PARAMS&);

std::vector<std::string> InitRefDataFilenames(int, char**);

#endif /* MERTCOMMON_H_ */
