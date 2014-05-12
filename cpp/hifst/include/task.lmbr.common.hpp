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

#ifndef TASK_LMBR_COMMON_HPP
#define TASK_LMBR_COMMON_HPP

/**
 * \file
 * \brief Common lmbr functions
 * \date 10-01-2013
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace lmbr {

///Interfaces with extractNGrams and generates information in the right format for lmbr classes
template<class Arc>
inline uint extractNGrams (fst::VectorFst<Arc> myfst,
                           std::vector<fst::NGramList>& ngramlist, uint minorder = 1, uint maxorder = 4) {
  if (!myfst.NumStates() ) return 0;
  std::vector<fst::NGram> tmplist;
  fst::extractNGrams<Arc> (myfst, tmplist, maxorder);
  if (ngramlist.size() < maxorder + 1)
    ngramlist.resize (maxorder + 1);
  for (std::vector<fst::NGram>::iterator it = tmplist.begin();
       it != tmplist.end();
       it++) {
    uint n = it->size();
    if (n >= minorder && n <= maxorder) {
      ngramlist[n][*it] = fst::StdArc::Weight::One();
    }
  }
  return tmplist.size();
};

}
} // end namespaces

#endif //TASK_LMBR_COMMON_HPP
