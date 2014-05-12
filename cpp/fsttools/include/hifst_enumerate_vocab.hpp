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

#ifndef HIFSTENUMERATE_HPP
#define HIFSTENUMERATE_HPP

/**
 * \file
 * \brief Extend EnumerateVocab to access kenlm ids.
 * \author Gonzalo Iglesias
 */

#include <lm/config.hh>
#include <lm/enumerate_vocab.hh>
#include <wordmapper.hpp>

namespace lm {

/**
 * \brief This class extends EnumerateVocab in kenlm code.
 * This class creates a grammar-integer to lm-integer hash
 * which will be used during composition. If there is an lm wordmap
 * available, then it uses the wordmap to synchronize.
 */
class HifstEnumerateVocab: public EnumerateVocab {
 private:
  typedef ucam::fsttools::IdBridge IdBridge;
  typedef ucam::util::WordMapper WordMapper;

  IdBridge& idb_;
  WordMapper *wm_;
 public:
  HifstEnumerateVocab (IdBridge& idb, WordMapper *wm) : idb_ (idb), wm_ (wm) {}
  virtual ~HifstEnumerateVocab() {}
  virtual void Add (WordIndex index, const StringPiece& str) {
    std::string s = str.as_string();
    unsigned t;
    LDEBUG ("Converting ... s=" << s << ",lm_idx=" << index);
    if (s == "<s>") t = 1;
    else if (s == "</s>") t = 2;
    else if (s == "<unk>") t = 0;
    else if (wm_ == NULL) t = ucam::util::toNumber<unsigned> (s);
    else {
      t = (*wm_) (s);
      LDEBUG ("Found gidx = " << t);
    }
    if (t < std::numeric_limits<unsigned>::max() )
      idb_.add (t, index);
  }
};
}

#endif
