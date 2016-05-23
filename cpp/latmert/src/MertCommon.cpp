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

#include "MertCommon.h"

MertOpt opts;

std::string Time() {
  time_t t;
  time (&t);
  char tmp[128];
  sprintf (tmp, "%s", ctime (&t) );
  tmp[strlen (tmp) - 1] = ' ';
  tmp[strlen (tmp)] = 0;
  return std::string (tmp);
}

void ReplacePattern (std::string& newname, std::string pattern, const char *pat,
                     std::string rep) {
  std::string::size_type pos = 0;
  newname = pattern;
  int i = strlen (pat);
  while ( (pos = newname.find (pat, pos) ) != std::string::npos) {
    newname.replace (pos, i, rep);
  }
}

void ReplacePattern (std::string& newname, std::string pattern, const char *pat,
                     float f) {
  std::ostringstream si;
  si << f;
  si.precision (4);
  ReplacePattern (newname, pattern, pat, si.str() );
}

void ReplacePattern (std::string& newname, std::string pattern, const char *pat,
                     int i) {
  std::ostringstream si;
  si << i;
  ReplacePattern (newname, pattern, pat, si.str() );
}

std::string ExpandPath (std::string pattern, const int idx) {
  ReplacePattern (pattern, pattern, "%idx%", idx);
  return pattern;
}

void InitializeFromLimits (std::vector<Sid>& ids, const std::string range) {
  unsigned int idxmin = 0;
  unsigned int idxmax = 0;
  sscanf (range.c_str(), "%d:%d", &idxmin, &idxmax);
  for (unsigned int idx = idxmin; idx <= idxmax; idx++) {
    ids.push_back (idx);
  }
}

void InitializeFromScript (std::vector<Sid>& ids, const std::string filename) {
  std::ifstream ifs (filename.c_str() );
  if (!ifs.good() ) {
    std::cerr << "ERROR: unable to open file " << filename << '\n';
    exit (1);
  }
  uint idx;
  while (ifs >> idx) {
    ids.push_back (idx);
  }
  ifs.close();
}

std::ostream& operator<< (std::ostream& o, const Sentence& s) {
  for (unsigned int i = 0; i < s.size(); ++i) {
    if (i > 0) {
      o << " ";
    }
    o << s[i];
  }
  return o;
}

static const char* CMD_LINE_TOKEN_FILE = "file:";
static const char* CMD_LINE_TOKEN_RANDOM = "random";

std::vector<PARAMS> InitializeVectorsFromFile (const string& filename) {
  std::vector<PARAMS> vws;
  std::ifstream ifs (filename.c_str() );
  if (!ifs.good() ) {
    std::cerr << "ERROR: unable to open file " << filename << '\n';
    exit (1);
  }
  std::string line;
  while (getline (ifs, line) ) {
    vws.push_back (fst::ParseParamString<double, std::vector<double> > (line) );
  }
  ifs.close();
  return vws;
}

// This may need to be revisited. Storing a single direction in a vector is incredibly wasteful of memory
// Probably should be replaced with some sort of sparse data structure
std::vector<PARAMS> InitializeVectorsFromAxes (unsigned int dim) {
  std::vector<PARAMS> vws;
  for (unsigned int k = 0; k < dim; ++k) {
    PARAMS axis (dim);
    for (unsigned int i = 0; i < dim; ++i) {
      axis[i] = 0;
    }
    axis[k] = 1;
    vws.push_back (axis);
  }
  return vws;
}

std::vector<PARAMS> InitializeVectorsFromRandom() {
  std::cerr << "ERROR: 'random' vector initialization not supported";
  exit (1);
}

std::vector<PARAMS> InitializeVectors (const std::string& pattern) {
  std::vector<PARAMS> vws;
  if (pattern.find (CMD_LINE_TOKEN_FILE) == 0) {
    std::string filename = pattern.substr (strlen (CMD_LINE_TOKEN_FILE) );
    vws = InitializeVectorsFromFile (filename);
  } //else if (pattern.find(CMD_LINE_TOKEN_RANDOM) == 0) {
  //  vws = InitializeVectorsFromRandom();
//  }
  else {
    vws.push_back (fst::ParseParamString<double, std::vector<double> > (pattern) );
  }
  return vws;
}

std::string ReadWeight (const std::string& filename) {
  std::ifstream ifs (filename.c_str() );
  if (!ifs.good() ) {
    std::cerr << "ERROR: unable to open file " << filename << '\n';
    exit (1);
  }
  std::string weight;
  ifs >> weight;
  ifs.close();
  return weight;
}

std::ostream& operator<< (std::ostream& strm, const PARAMS& vw) {
  char separator = ',';
  for (uint k = 0; k < vw.size(); ++k) {
    if (k > 0) {
      strm << separator;
    }
    strm << vw[k];
  }
  return strm;
}

PARAMS operator- (const PARAMS& vw1, const PARAMS& vw2) {
  if (vw1.size() != vw2.size() ) {
    std::cerr << "Cannot subtract two vectors of different sizes. V1: "
         << vw1.size() << " V2:" << vw2.size() << std::endl;
    exit (1);
  }
  PARAMS vv (vw1.size() );
  for (unsigned int k = 0; k < vw1.size(); ++k) {
    vv[k] = vw1[k] - vw2[k];
  }
  return vv;
}

PARAMS operator+ (const PARAMS& vw1, const PARAMS& vw2) {
  if (vw1.size() != vw2.size() ) {
    std::cerr << "Cannot sum two vectors of different sizes. V1: " << vw1.size()
         << " V2:" << vw2.size() << std::endl;
    exit (1);
  }
  PARAMS vv (vw1.size() );
  for (unsigned int k = 0; k < vw1.size(); ++k) {
    vv[k] = vw1[k] + vw2[k];
  }
  return vv;
}

template double fst::DotProduct<double> (
  const fst::TropicalSparseTupleWeight<double>&, const PARAMS&);

std::vector<std::string> InitRefDataFilenames (int argc, char** argv) {
  std::vector<std::string> refFilenames;
  if (argc == 1) {
    std::cerr << "ERROR: no reference files specified\n";
    exit (1);
  } else {
    tracer << argc - 1 << " reference files specified:\n";
    for (int i = 1; i < argc; ++i) {
      tracer << "r[" << i << "]=" << argv[i] << '\n';
      refFilenames.push_back (argv[i]);
    }
  }
  return refFilenames;
}
