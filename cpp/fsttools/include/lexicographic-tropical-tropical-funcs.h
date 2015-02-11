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

#ifndef LEXICOGRAPHIC_TROPICAL_TROPICAL_FUNCS_H
#define LEXICOGRAPHIC_TROPICAL_TROPICAL_FUNCS_H

/**
 * \file
 * \brief Convenience functors/functions for lexicographic<tropical,tropical> semiring
 * \date October 2012
 * \author Gonzalo Iglesias
 */

namespace fst {

template<class Arc>
struct  GetWeight {
  float operator ()  ( const typename Arc::Weight& weight ) {
    return weight.Value();
  }
  //  float getNotConst( typename Arc::Weight weight ) { return weight.Value();}
};

template<>
struct  GetWeight<LexStdArc> {
  float operator ()  ( const LexStdArc::Weight& weight ) {
    return weight.Value1().Value();
  }
  //  float getNotConst( LexStdArc::Weight weight ) { return weight.Value1().Value();}
};

/**
 * \brief Templated functor that creates a weight given a float.
 */
template<class Arc>
struct  MakeWeight {
  inline typename Arc::Weight operator ()  ( const float weight )  const {
    return typename Arc::Weight ( weight );
  };
  inline typename Arc::Weight operator ()  ( const typename Arc::Weight& weight )
  const  {
    return weight ;
  };
  inline void update() {};
};

/**
 * \brief Specialized implementation of MakeWeight for lexicographic semiring over two tropical weights. Second weight is set to One().
 */
template<>
struct  MakeWeight<LexStdArc> {
  inline LexStdWeight operator () ( const float weight )  {
    return LexStdWeight ( weight, StdArc::Weight::One() );
  };
  inline LexStdWeight operator () ( const StdArc::Weight& weight )  {
    return LexStdWeight ( weight.Value(), StdArc::Weight::One() );
  };
  inline LexStdWeight operator () ( const LexStdWeight& weight )  {
    return LexStdWeight ( weight.Value1(), StdArc::Weight::One() );
  };
  inline void update() {};
};

/**
 * \brief Templated functor that creates a weight given a float.
 */

template<class Arc>
struct  MakeWeight2 {
  typedef typename Arc::Weight Weight;
  inline Weight operator () (float const weight )  const {
    return Weight ( weight );
  };
  inline Weight operator () (Weight const & weight) const  {

    return weight ;
  };
  inline void update() {};
};

/**
 * \brief Specialized implementation of MakeWeight2 for lexicographic semiring over two tropical weights. Second weight is set to the same input weight.
 */
template<>
struct  MakeWeight2<LexStdArc> {
  inline const LexStdWeight operator () ( const float weight )  const {
    return LexStdWeight ( weight, weight );
  };
  inline const LexStdWeight operator () ( const StdArc::Weight& weight )  const {
    return LexStdWeight ( weight.Value(), weight.Value() );
  };
  inline const LexStdWeight operator () ( const LexStdWeight& weight )  const {
    return LexStdWeight ( weight.Value2(), weight.Value2() );
  };
  inline void update() {};
};

/**
 * \brief Function object that applies to every single weight a scaling factor
 */
template<class Arc>
struct  ScaleWeight {
  ScaleWeight ( float scale ) :
    scale_ ( scale ) {
  };

  inline typename Arc::Weight operator () ( const float weight )  const {
    return typename Arc::Weight ( weight * scale_ );
  };
  inline typename Arc::Weight operator ()  ( const typename Arc::Weight& weight )
  const  {
    return weight.Value() * scale_;
  };

  float scale_;

};

/**
 * \brief Template specialization for LexStdArc of ScaleWeight
 */

template<>
struct  ScaleWeight<LexStdArc> {
  ScaleWeight ( float scale ) :
    scale_ ( scale ) {
  };
  inline const LexStdWeight operator () ( const float weight )  const {
    return LexStdWeight ( weight * scale_, weight * scale_ );
  };
  inline const LexStdWeight operator () ( const LexStdWeight& weight )  const {
    return LexStdWeight ( scale_ * weight.Value1().Value(),
                          scale_ * weight.Value2().Value() );
  };
  float scale_;
};

struct  LexToStd {
  inline StdArc::Weight operator () ( const LexStdWeight& weight ) const  {
    return StdArc::Weight ( weight.Value1() );
  };
};

}; //namespace fst

#endif
