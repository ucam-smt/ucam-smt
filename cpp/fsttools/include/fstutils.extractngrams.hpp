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

#ifndef FSTUTILS_EXTRACTNGRAMS_HPP
#define FSTUTILS_EXTRACTNGRAMS_HPP

namespace fst {

typedef unsigned WordId;
typedef std::basic_string<WordId> NGram;
typedef std::tr1::unordered_map<NGram
, StdArc::Weight
, ucam::util::hashfvecuint
, ucam::util::hasheqvecuint> NGramList;
typedef std::vector<NGram> NGramVector;

template<class Arc>
inline void filterTransducerByLength (VectorFst<Arc>& myfst, unsigned min,
                                      unsigned max) {
  // Create size limiting fst
  VectorFst<StdArc> sizefst;
  sizefst.AddState();
  sizefst.SetStart (0);
  for (unsigned k = 1; k < min; ++k) {
    sizefst.AddState();
    sizefst.AddArc (k - 1, StdArc (RHO, RHO, 0, k) );
  }
  for (unsigned k = min; k <= max; ++k) {
    sizefst.AddState();
    sizefst.SetFinal (k, StdArc::Weight::One() );
    sizefst.AddArc (k - 1, StdArc (RHO, RHO, 0, k) );
  }
  myfst = (RRhoCompose (myfst, sizefst) );
}

/**
 * \brief Functor with recursive procedure that extracts into a vector all the possible ngrams of a lattice,
 *  up to a given order, and starting from a given state.
 */

template<class Arc>
class GetNGrams {
 private:
  NGram v;
 public:

  inline void operator() (std::vector<NGram>& ngrams, const VectorFst<Arc>& count,
                          unsigned order, typename Arc::StateId s = 0) {
    if (!order) return;
    for (ArcIterator< VectorFst<Arc> > i (count, s); !i.Done(); i.Next() ) {
      Arc a = i.Value();
      v.push_back (a.ilabel);
      if (count.Final (a.nextstate) != Arc::Weight::Zero() )  ngrams.push_back ( v );
      (*this) (ngrams, count, order - 1, a.nextstate);
      v.resize (v.size() - 1);
    }
  }
};

template<class Arc>
inline void extractNGrams (VectorFst<Arc>& myfst, std::vector<NGram>& ngrams,
                           unsigned maxorder) {
  if (!myfst.NumStates() ) return;
  LINFO ("Building substring transducer");
  buildSubstringTransducer (&myfst);
  LDBG_EXECUTE (FstWrite (myfst, "fsts/extractngrams/ss.fst") );
  LINFO ("Filtering transducer by maxorder");
  filterTransducerByLength (myfst, 1, maxorder);
  LDBG_EXECUTE (FstWrite (myfst, "fsts/extractngrams/ss+maxorder.fst") );
  LINFO ("Determinizing...");
  Determinize (myfst, &myfst);
  LDBG_EXECUTE (FstWrite (myfst, "fsts/extractngrams/ss+maxorder+det.fst") );
  LINFO ("Arcsorting...");
  ArcSort (&myfst, StdILabelCompare() );
  LDBG_EXECUTE (FstWrite (myfst, "fsts/extractngrams/ss+maxorder+det+as.fst") );
  GetNGrams<Arc>() (ngrams, myfst, maxorder,
                    myfst.Start() ); //call recursive function to store ngrams
}

} // end namespaces

inline std::ostream& operator<< (std::ostream& o, const fst::NGram& n) {
  for (int i = 0; i < n.size(); i++) {
    if (i > 0) {
      o << " ";
    }
    o << n[i];
  }
  return o;
}

#endif  //FSTUTILS_EXTRACTNGRAMS_HPP
