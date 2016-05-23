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

#include "RefsData.h"

RefsData::RefsData() {};

RefsData RefsData::dummy;

RefsData::RefsData (const std::vector<Sentence>& refs) {
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    std::vector<NGramToCountMap> tmpCounts (refs.size() );
    for (unsigned int k = 0; k < refs.size(); ++k) {
      int refssize_minus_n = refs[k].size() -
                             n; // needs to be an int, not a unsigned int in case it is negative
      for (int i = 0; i < refssize_minus_n; ++i) {
        NGram u = SubStr (refs[k], i, n);
        refCounts[u] = 1;
        tmpCounts[k][u]++;
      }
    }
    for (unsigned int k = 0; k < refs.size(); ++k) {
      for (NGramToCountMap::iterator it = refCounts.begin();
           it != refCounts.end(); ++it) {
        it->second = max (it->second, tmpCounts[k][it->first]);
      }
    }
  }
  for (unsigned int k = 0; k < refs.size(); ++k) {
    refLengths.push_back (refs[k].size() );
  }
  sort (refLengths.begin(), refLengths.end() );
}

BleuStats RefsData::ComputeBleuStats (const Sentence& hyp) const {
  BleuStats bs;
  bs.refLength = ClosestReferenceLength (hyp.size() );
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER && n < hyp.size();
       ++n) {
    NGramToCountMap hypCounts;
    if (hyp.size() > n) {
      bs.tots[n] = hyp.size() - n;
    }
    for (unsigned int i = 0; i < hyp.size() - n; ++i) {
      hypCounts[SubStr (hyp, i, n)]++;
    }
    for (NGramToCountMap::const_iterator hit = hypCounts.begin();
         hit != hypCounts.end(); ++hit) {
      NGramToCountMap::const_iterator rit = refCounts.find (hit->first);
      bs.hits[n] += min (rit == refCounts.end() ? 0 : rit->second,
                         hit->second); // Sum clipped counts
    }
  }
  return bs;
}

NGram RefsData::SubStr (const Sentence& s, const unsigned int n,
                        const unsigned int l) const {
  return NGram (s.begin() + n, s.begin() + n + l + 1);
}

unsigned int RefsData::ClosestReferenceLength (
  const unsigned int hypLength) const {
  unsigned int refLength = 0;
  unsigned int refDistance = numeric_limits<unsigned int>::max();
  for (unsigned int k = 0; k < refLengths.size(); ++k) {
    unsigned int distance = abs ( (int) refLengths[k] - (int) hypLength);
    if (distance < refDistance) {
      refDistance = distance;
      refLength = refLengths[k];
    }
  }
  return refLength;
}

std::string RefsData::ToString (const char separator = ' ') const {
  ostringstream oss;
  for (NGramToCountMap::const_iterator it = refCounts.begin();
       it != refCounts.end(); ++it) {
    NGram u = it->first;
    unsigned int count = it->second;
    for (int w = 0; w < u.size(); ++w) {
      if (w > 0) {
        oss << " ";
      }
      oss << u[w];
    }
    oss << '\t' << count << separator;
  }
  return oss.str();
}

std::vector<std::vector<std::string> > LoadRefFiles (std::vector<std::string>
    files) {
  tracer << "loading and initializing reference ngrams...\n";
  std::vector<std::vector<std::string> > refsStr (files.size() );
  for (unsigned int r = 0; r < files.size(); ++r) {
    std::ifstream ifs (files[r].c_str() );
    if (!ifs.good() ) {
      std::cerr << "ERROR: unable to open file " << files[r] << '\n';
      exit (1);
    }
    std::string line;
    refsStr[r].push_back ("");
    while (getline (ifs, line) ) {
      refsStr[r].push_back (line);
    }
    if (opts.verbose) {
      tracer << "loaded " << refsStr[r].size() - 1 << " translations from "
             << files[r] << '\n';
    }
    ifs.close();
  }
  return refsStr;
}

void IntegerEncRefs::LoadRefData (std::vector<std::string> files) {
  std::vector<std::vector<std::string> > refsStr = LoadRefFiles (files);
  refsData.push_back (RefsData::dummy);
  for (unsigned int sid = 0; sid < refsStr[0].size() - 1 ; ++sid) {
    std::vector<Sentence> refsIdx;
    for (unsigned int r = 0; r < files.size(); ++r) {
      std::istringstream iss (refsStr[r][sid + 1]);
      Sentence s;
      Wid w;
      while (iss >> w) {
        s.push_back (w);
      }
      refsIdx.push_back (s);
    }
    refsData.push_back (RefsData (refsIdx) );
  }
}

IntegerEncRefs::ErrorStats IntegerEncRefs::ComputeErrorStats (Sid sid,
    Sentence h) const {
  if (sid + 1 > refsData.size() ) {
    std::cerr << "ERROR: no references loaded for sentence s=" << sid << '\n';
    exit (1);
  }
  return refsData[sid].ComputeBleuStats (h);
}
