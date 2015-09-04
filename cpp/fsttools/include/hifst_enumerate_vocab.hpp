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
 * Note: The nplm wrapper does not support this functionality.
 */
template<class WordMapperT>
class HifstEnumerateVocab: public EnumerateVocab {
 private:
  typedef ucam::fsttools::IdBridge IdBridge;
  WordMapperT *wm_;
 public:
  // we need this available for a quick nplm hack
  IdBridge& idb_;
  HifstEnumerateVocab (IdBridge& idb, WordMapperT *wm) : idb_ (idb), wm_ (wm) {}
  virtual ~HifstEnumerateVocab() {}
  virtual void Add (WordIndex index, const StringPiece& str) {
    std::string s = str.as_string();
    Add(index, s);
  }

  void Add (WordIndex index, const std::string& s) {
    unsigned t;
    LDEBUG ("Converting ... s=" << s << ",lm_idx=" << index);
    if (s == "<s>") t = 1;
    else if (s == "</s>") t = 2;
    else if (s == "<unk>") t = 0;
#ifdef WITH_NPLM
    else if (s == "<null>") t = 3;
#endif
    else if (wm_ == NULL) t = ucam::util::toNumber<unsigned> (s);
    else {
      t = (*wm_) (s);
      LDEBUG ("Found gidx = " << t);
    }
    if (t < std::numeric_limits<unsigned>::max() ) {
      LDEBUG ("Adding " << t << " => " << index);
      idb_.add (t, index);
    }
  }

  void AddOutput (WordIndex index, const std::string& s) {
    unsigned t;
    LDEBUG ("Converting ... s=" << s << ",lm_idx=" << index);
    if (s == "<s>") t = 1;
    else if (s == "</s>") t = 2;
    else if (s == "<unk>") t = 0;
#ifdef WITH_NPLM
    else if (s == "<null>") t = 3;
#endif
    else if (wm_ == NULL) t = ucam::util::toNumber<unsigned> (s);
    else {
      t = (*wm_) (s);
      LDEBUG ("Found gidx = " << t);
    }
    if (t < std::numeric_limits<unsigned>::max() ) {
      LDEBUG ("Adding " << t << " => " << index);
      idb_.addOutput (t, index);
    }
  }
};
}

#endif
