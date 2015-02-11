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

#ifndef TROPICALSPARSETUPLEWEIGHT_H_
#define TROPICALSPARSETUPLEWEIGHT_H_

/**
 * \file
 * \brief Implementation of tropical sparse tuple weight semiring.
 * \date 10-10-2012
 * \author Rory Waite
 */

# include <params.hpp>

namespace fst {

/**
 * \brief Implements Tropical Sparse tuple weight semiring, extending from openfst SparsePowerWeight class
 * \remark Main addition: The dot product of features with its scales is a tropical weight.
 */

template<typename T>
class TropicalSparseTupleWeight: public
  SparsePowerWeight<TropicalWeightTpl<T> > {

 public:

  static std::vector<T>& Params() {
    static ucam::util::ParamsInit<T> params;
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
    this->SetDefaultValue ( W::One() );
  }

  TropicalSparseTupleWeight ( const SparsePowerWeight<W>& sw ) :
    SparsePowerWeight<W> ( sw ) {
  }

  TropicalSparseTupleWeight ( W w ) :
    SparsePowerWeight<W> ( w ) {
  }

  inline static std::string GetPrecisionString() {
    int64 size = sizeof ( T );
    if ( size == sizeof ( float ) ) {
      return "";
    }
    size *= CHAR_BIT;
    std::string result;
    Int64ToStr ( size, &result );
    return result;
  }

  static const std::string& Type() {
    static const std::string type = "tropicalsparsetuple"
                                    + TropicalSparseTupleWeight<T>::GetPrecisionString();
    return type;
  }

  static uint64 Properties() {
    return W::Properties()
           & ( kLeftSemiring | kRightSemiring | kCommutative | kIdempotent
               | kPath );
  }

  static const TropicalSparseTupleWeight<T>& Zero() {
    static TropicalSparseTupleWeight<T> zero ( W::Zero() );
    return zero;
  }

  static const TropicalSparseTupleWeight<T>& One() {
    static TropicalSparseTupleWeight<T> one ( W::One() );
    return one;
  }

  TropicalSparseTupleWeight<T> Quantize ( float delta = kDelta ) const {
    TropicalSparseTupleWeight<T> w = SparsePowerWeight<W>::Quantize ( delta );
    w.SetDefaultValue ( this->DefaultValue() );
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

///Implements Dot product of two vector weights
template<typename T>
T DotProduct ( const TropicalSparseTupleWeight<T>& w,
               const std::vector<T>& vw ) {
  T result = w.DefaultValue().Value();
  for ( SparseTupleWeightIterator<TropicalWeightTpl<T>, int> it ( w ); !it.Done();
        it.Next() ) {
    T param;
    // Special case for flat params
    if ( vw.empty() ) {
      param = 1;
    } else if ( it.Value().first > int ( vw.size() ) ) {
      cerr
          << "feature vector has a larger dimensionality than the parameters. "
          << "Params: " << vw.size() << " Features: "
          << it.Value().first << endl;
      exit ( 1 );
    } else if (it.Value().first <  0) // ignore negative indices
      param = 0;
    else {
      param = vw[it.Value().first - 1];
    }
    result += it.Value().second.Value() * param;
  }
  return result;
}

template<class T>
inline TropicalSparseTupleWeight<T> Plus (
  const TropicalSparseTupleWeight<T>& vw1,
  const TropicalSparseTupleWeight<T>& vw2 ) {
  T w1 = DotProduct ( vw1, TropicalSparseTupleWeight<T>::Params() );
  T w2 = DotProduct ( vw2, TropicalSparseTupleWeight<T>::Params() );
  return w1 < w2 ? vw1 : vw2;
}

template<typename T>
inline bool ApproxEqual ( const TropicalSparseTupleWeight<T>& vw1,
                          const TropicalSparseTupleWeight<T>& vw2, float delta = kDelta ) {
  const SparsePowerWeight<TropicalWeightTpl<T> >& spw1 = vw1;
  const SparsePowerWeight<TropicalWeightTpl<T> >& spw2 = vw2;
  return ApproxEqual ( spw1, spw2, delta );
}

template<class T>
inline TropicalSparseTupleWeight<T> Times (
  const TropicalSparseTupleWeight<T>& w1,
  const TropicalSparseTupleWeight<T>& w2 ) {
  TropicalSparseTupleWeight<T> ret;
  SparseTupleWeightTimesMapper<TropicalWeightTpl<T>, int> operator_mapper;
  SparseTupleWeightMap ( &ret, w1, w2, operator_mapper );
  return ret;
}

// Semimodule divide operation
template<class T>
inline TropicalSparseTupleWeight<T> Divide (
  const TropicalSparseTupleWeight<T>& w1,
  const TropicalSparseTupleWeight<T>& w2, DivideType type = DIVIDE_ANY ) {
  TropicalSparseTupleWeight<T> ret;
  SparseTupleWeightDivideMapper<TropicalWeightTpl<T>, int> operator_mapper (
    type );
  SparseTupleWeightMap ( &ret, w1, w2, operator_mapper );
  return ret;
};

/*

template<typename W>
struct StdToTropicalSparseMapper {
  explicit StdToTropicalSparseMapper(uint k_ = 1) :
      k_(k_) {
  }
  ;

  TropicalSparseTupleWeight<W> operator()(W w) {
    TropicalSparseTupleWeight<W> sparse;
    sparse.push(k_, w);
    return sparse;
  }

private:
  uint k_;
};

struct Expand {

  TropicalSparseTupleWeight<double> operator()(
      const TropicalSparseTupleWeight<float>& w32) const {
    TropicalSparseTupleWeight<double> result;
    for (SparseTupleWeightIterator<TropicalWeight, int> it(w32); !it.Done();
        it.Next()) {
      result.Push(it.Value().first,
          TropicalWeightTpl<double>(it.Value().second.Value()));
    }
    return result;

  }
}
;

typedef GeneralMapper<ArcTpl<TropicalSparseTupleWeight<float> >,
    ArcTpl<TropicalSparseTupleWeight<double> >, Expand> ExpandMapper;

*/

///Map functor used with generic weight mapper
template<typename T>
struct DotProductMap {
  DotProductMap ( const std::vector<T>& param ) :
    param ( param ) {
  }

  inline TropicalWeightTpl<T> operator() ( const TropicalSparseTupleWeight<T>& w )
  const {
    return DotProduct ( w, param );
  }

 private:
  std::vector<T> param;
};

///Functor to convert sparse tuple weight to tropical (single weight)
template<typename T>
struct VectorToStd {

  VectorToStd ( int32_t k = 1 ) :
    k ( k ) {
  }

  TropicalWeightTpl<T> operator() ( const TropicalSparseTupleWeight<T>& w )
  const {
    for ( SparseTupleWeightIterator<TropicalWeightTpl<T>, int> it ( w );
          !it.Done(); it.Next() ) {
      if ( it.Value().first - 1 == k ) {
        return it.Value().second;
      }
    }
    return w.DefaultValue();
  }

 private:
  int32_t k;
};

///Functor that converts tropical to sparse tuple weight
template<typename T>
struct StdToVector {

  StdToVector ( int32_t k ) :
    k ( k ) {
  }

  TropicalSparseTupleWeight<T> operator() ( const TropicalWeightTpl<T>& w )
  const {
    TropicalSparseTupleWeight<T> result;
    result.Push ( k + 1, w.Value() );
    return result;
  }
 private:
  int32_t k;
};

//typedef GenericWeightMapper< ArcTpl<TropicalWeightTpl<float> > , ArcTpl<TropicalSparseTupleWeight<float> >, StdToVector<float> > StdToSparseMapper;
//typedef GenericWeightMapper<ArcTpl<TropicalSparseTupleWeight<float> >,ArcTpl<TropicalWeightTpl<float> >, DotProductMap<float> > DotProduct32Mapper;
//typedef GenericWeightMapper<ArcTpl<TropicalSparseTupleWeight<double> >,ArcTpl<TropicalWeightTpl<double> >, DotProductMap<double> > DotProductMapper;
//typedef GenericWeightMapper<ArcTpl<TropicalSparseTupleWeight<float> >,ArcTpl<TropicalWeightTpl<float> >, VectorToStd<float> > SparseToStdMapper;

} //namespace fst

#endif /* TROPICALSPARSETUPLEWEIGHT_H_ */
