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

#ifndef TROPICALSPARSETUPLEWEIGHT_H_
#define TROPICALSPARSETUPLEWEIGHT_H_

#include <fst/fstlib.h>
#include <vector>
#include "params.h"

namespace fst {

template<typename T>
class TropicalSparseTupleWeight: public
  SparsePowerWeight<TropicalWeightTpl<T> > {

 public:

  static std::vector<T>& Params() {
    static ParamsInit<T> params;
    return params.params;
  }

  typedef TropicalWeightTpl<T> W;

  using SparsePowerWeight<W>::Zero;
  using SparsePowerWeight<W>::One;
  using SparsePowerWeight<W>::Quantize;
  using SparsePowerWeight<W>::Reverse;
  using SparsePowerWeight<W>::Type;
  using SparsePowerWeight<W>::Properties;

  typedef TropicalSparseTupleWeight<T> ReverseWeight;

  TropicalSparseTupleWeight() :
    SparsePowerWeight<W>() {
    this->SetDefaultValue (W::One() );
  }

  TropicalSparseTupleWeight (const SparsePowerWeight<W>& sw) :
    SparsePowerWeight<W> (sw) {
  }

  TropicalSparseTupleWeight (W w) :
    SparsePowerWeight<W> (w) {
  }

  inline static std::string GetPrecisionString() {
    int64 size = sizeof (T);
    if (size == sizeof (float) ) {
      return "";
    }
    size *= CHAR_BIT;
    std::string result;
    Int64ToStr (size, &result);
    return result;
  }

  static const std::string& Type() {
    static const std::string type = "tropicalsparsetuple"
                                    + TropicalSparseTupleWeight<T>::GetPrecisionString();
    return type;
  }

  static uint64 Properties() {
    return W::Properties()
           & (kLeftSemiring | kRightSemiring | kCommutative | kIdempotent
              | kPath);
  }

  static const TropicalSparseTupleWeight<T>& Zero() {
    static TropicalSparseTupleWeight<T> zero (W::Zero() );
    return zero;
  }

  static const TropicalSparseTupleWeight<T>& One() {
    static TropicalSparseTupleWeight<T> one (W::One() );
    return one;
  }

  TropicalSparseTupleWeight<T> Quantize (float delta = kDelta) const {
    TropicalSparseTupleWeight<T> w = SparsePowerWeight<W>::Quantize (delta);
    w.SetDefaultValue (this->DefaultValue() );
    return w;
  }

  ReverseWeight Reverse() const {
    return *this;
  }

  template<typename TT>
  friend TropicalSparseTupleWeight<TT> Plus (
    const TropicalSparseTupleWeight<TT>&,
    const TropicalSparseTupleWeight<TT>&);
};

template<typename T>
T DotProduct (const TropicalSparseTupleWeight<T>& w, const std::vector<T>& vw) {
  T result = w.DefaultValue().Value();
  for (SparseTupleWeightIterator<TropicalWeightTpl<T>, int> it (w); !it.Done();
       it.Next() ) {
    T param;
    // Special case for flat params
    if (vw.empty() ) {
      param = 1;
    } else if (it.Value().first > int (vw.size() ) ) {
      cerr
          << "feature vector has a larger dimensionality than the parameters. "
          << "Params: " << vw.size() << " Features: "
          << it.Value().first << endl;
      exit (1);
    } else {
      param = vw[it.Value().first - 1];
    }
    result += it.Value().second.Value() * param;
  }
  return result;
}

template<typename T>
struct AE<T, TropicalSparseTupleWeight<T> > {
  static void AddElement (TropicalSparseTupleWeight<T>& params ,
                          unsigned int index
                          , T param) {
    params.Push (index + 1, param);
  }
};

template<class T>
inline TropicalSparseTupleWeight<T> Plus (
  const TropicalSparseTupleWeight<T>& vw1,
  const TropicalSparseTupleWeight<T>& vw2) {
  T w1 = DotProduct (vw1, TropicalSparseTupleWeight<T>::Params() );
  T w2 = DotProduct (vw2, TropicalSparseTupleWeight<T>::Params() );
  return w1 < w2 ? vw1 : vw2;
}

template<typename T>
inline bool ApproxEqual (const TropicalSparseTupleWeight<T>& vw1,
                         const TropicalSparseTupleWeight<T>& vw2, float delta = kDelta) {
  const SparsePowerWeight<TropicalWeightTpl<T> >& spw1 = vw1;
  const SparsePowerWeight<TropicalWeightTpl<T> >& spw2 = vw2;
  return ApproxEqual (spw1, spw2, delta);
}

template<class T>
inline TropicalSparseTupleWeight<T> Times (
  const TropicalSparseTupleWeight<T>& w1,
  const TropicalSparseTupleWeight<T>& w2) {
  TropicalSparseTupleWeight<T> ret;
  SparseTupleWeightTimesMapper<TropicalWeightTpl<T>, int> operator_mapper;
  SparseTupleWeightMap (&ret, w1, w2, operator_mapper);
  return ret;
}

// Semimodule divide operation
template<class T>
inline TropicalSparseTupleWeight<T> Divide (
  const TropicalSparseTupleWeight<T>& w1,
  const TropicalSparseTupleWeight<T>& w2, DivideType type = DIVIDE_ANY) {
  TropicalSparseTupleWeight<T> ret;
  SparseTupleWeightDivideMapper<TropicalWeightTpl<T>, int> operator_mapper (
    type);
  SparseTupleWeightMap (&ret, w1, w2, operator_mapper);
  return ret;
}

template<typename FromArc, typename ToArc, typename M>
struct GeneralMapper {

  GeneralMapper (M& m) :
    m (m) {
  }
  ;

  ToArc operator() (const FromArc& arc) const {
    if (arc.weight == FromArc::Weight::Zero() ) {
      return ToArc (arc.ilabel, arc.olabel, ToArc::Weight::Zero(),
                    arc.nextstate);
    }
    if (arc.weight == FromArc::Weight::One() ) {
      return ToArc (arc.ilabel, arc.olabel, ToArc::Weight::One(),
                    arc.nextstate);
    }
    return ToArc (arc.ilabel, arc.olabel, m (arc.weight), arc.nextstate);
  }

  MapSymbolsAction InputSymbolsAction() const {
    return MAP_COPY_SYMBOLS;
  }
  MapSymbolsAction OutputSymbolsAction() const {
    return MAP_COPY_SYMBOLS;
  }
  MapFinalAction FinalAction() const {
    return MAP_NO_SUPERFINAL;
  }
  uint Properties (uint props) const {
    return (props & kWeightInvariantProperties) | kUnweighted;
  }

 private:
  M m;
};

template<typename W>
struct StdToTropicalSparseMapper {
  explicit StdToTropicalSparseMapper (uint k_ = 1) :
    k_ (k_) {
  }
  ;

  TropicalSparseTupleWeight<W> operator() (W w) {
    TropicalSparseTupleWeight<W> sparse;
    sparse.put (k_, w);
    return sparse;
  }

 private:
  uint k_;
};

struct Expand {

  TropicalSparseTupleWeight<double> operator() (
    const TropicalSparseTupleWeight<float>& w32) const {
    TropicalSparseTupleWeight<double> result;
    for (SparseTupleWeightIterator<TropicalWeight, int> it (w32); !it.Done();
         it.Next() ) {
      result.Push (it.Value().first,
                   TropicalWeightTpl<double> (it.Value().second.Value() ) );
    }
    return result;
  }
}
;

typedef GeneralMapper<ArcTpl<TropicalSparseTupleWeight<float> >,
        ArcTpl<TropicalSparseTupleWeight<double> >, Expand> ExpandMapper;

template<typename T>
struct DotProductMap {
  DotProductMap (const std::vector<T>& param) :
    param (param) {
  }

  TropicalWeightTpl<T> operator() (
    const TropicalSparseTupleWeight<T>& w) const {
    return DotProduct (w, param);
  }

 private:
  std::vector<T> param;
};

template<typename T>
struct VectorToStd {

  VectorToStd (int32_t k) :
    k (k) {
  }

  TropicalWeightTpl<T> operator() (const TropicalSparseTupleWeight<T>& w) const {
    for (SparseTupleWeightIterator<TropicalWeightTpl<T>, int> it (w);
         !it.Done(); it.Next() ) {
      if (it.Value().first - 1 == k) {
        return it.Value().second;
      }
    }
    return w.DefaultValue();
  }

 private:
  int32_t k;
};

//typedef GeneralMapper<ArcTpl<TropicalSparseTupleWeight<float> >,
//    ArcTpl<TropicalWeightTpl<float> >, VectorToStd<float> > SparseToStdMapper;

template<typename T>
struct StdToVector {

  StdToVector (int32_t k) :
    k (k) {
  }

  TropicalSparseTupleWeight<T> operator() (const TropicalWeightTpl<T>& w) const {
    TropicalSparseTupleWeight<T> result;
    result.Push (k + 1, w.Value() );
    return result;
  }

 private:
  int32_t k;
};

// typedef GeneralMapper<ArcTpl<TropicalWeightTpl<float> >,
//   ArcTpl<TropicalSparseTupleWeight<float> >, StdToVector<float> > StdToSparseMapper;

} // namespace fst

typedef fst::GeneralMapper<fst::ArcTpl<fst::TropicalSparseTupleWeight<float> >,
        fst::ArcTpl<fst::TropicalWeightTpl<float> >, fst::VectorToStd<float> >
        SparseToStdMapper;

typedef fst::GeneralMapper<fst::ArcTpl<fst::TropicalWeightTpl<float> >,
        fst::ArcTpl<fst::TropicalSparseTupleWeight<float> >, fst::StdToVector<float> >
        StdToSparseMapper;

#endif /* TROPICALSPARSETUPLEWEIGHT_H_ */
