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

#ifndef TROPICALSPARSETUPLEWEIGHT_MAKEWEIGHT_H_
#define TROPICALSPARSETUPLEWEIGHT_MAKEWEIGHT_H_

/**
 * \file
 * \brief Convenience functors that allow transparent handling for weights within hifst.
 * \date 11-10-2012
 * \author Gonzalo Iglesias
 */

namespace fst {

///Templated Functor to generate a sparse vector weight from any other representation
template<typename Arc = StdArc>
struct MakeSparseVectorWeight {
  explicit MakeSparseVectorWeight ( int32_t k ) :
    k ( k ) {
  }

  inline TropicalSparseTupleWeight<float> operator() ( const typename Arc::Weight&
      w ) const {
    TropicalSparseTupleWeight<float> result;
    result.Push ( k + 1, w.Value() );
    return result;
  }

 private:
  int32_t k;
};

///Template specialization of MakeSparseVectorWeight  functor for LexStdArc.
template<>
struct MakeSparseVectorWeight<LexStdArc> {
  explicit MakeSparseVectorWeight ( int32_t k ) :
    k ( k ) {
  }

  inline TropicalSparseTupleWeight<float> operator() ( const  LexStdArc::Weight&
      w ) const {
    TropicalSparseTupleWeight<float> result;
    result.Push ( k + 1, w.Value1().Value() );
    return result;
  }
 private:
  int32_t k;
};

///Template specialization of functor MakeWeight for TupleArc32. See lexicographic-tropical-tropical-funcs.h
template<>
struct MakeWeight<TupleArc32> {
  ///It is specially important that k defaults to 0 in this case, as MakeWeight is used by applylanguagemodeltask to transparently handle different arcs.
  explicit MakeWeight ( int32_t k = 0 ) :
    k_ ( k ) {
  };

  ///Creates a weight given a float.
  inline TupleArc32::Weight operator () ( const float weight )  const {
    TropicalSparseTupleWeight<float> result;
    result.Push ( k_ + 1, weight );
    return result;
  };
  ///Copies the weight through
  inline TupleArc32::Weight operator ()  ( const TupleArc32::Weight& weight )
  const  {
    return weight ;
  };

  ///Increases index. Provides a way to do sequentially transparent operations, e.g. language model composition with three language models (see fstutils.applylmonthefly.hpp).
  inline void update() {
    ++k_;
  };
  inline int32_t get_k() {
    return k_;
  };
 private:
  int32_t k_;
};

} //namespace fst

#endif /* TROPICALSPARSETUPLEWEIGHT_MAKEWEIGHT_H_ */
