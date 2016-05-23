// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use these files except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copyright 2012 - Gonzalo Iglesias, Adrià de Gispert, William Byrne

#ifndef DATA_LMBR_HPP
#define DATA_LMBR_HPP

#define tracer cerr

namespace ucam {
namespace lmbr {

const unsigned kEpsLabel = 0;

typedef std::unordered_set<fst::WordId> Wlist;
typedef Wlist::iterator WlistIt;
typedef double Posterior;
typedef unordered_map< fst::NGram
, fst::StdArc::StateId
, ucam::util::hashfvecuint
, ucam::util::hasheqvecuint> NGramToStateMapper;
typedef unordered_map< fst::NGram
, std::vector< std::vector<Posterior> >
, ucam::util::hashfvecuint
, ucam::util::hasheqvecuint> NGramToPosteriorsMapper;

struct lmbrtunedata {
  std::basic_string<float> wps;
  std::basic_string<float> alpha;
  unsigned idx;
  std::vector<string> hyp;
};

}
} // end namespaces

#endif  // DATA_LMBR_HPP
