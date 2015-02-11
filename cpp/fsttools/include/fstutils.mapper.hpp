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

#ifndef FSTUTILS_MAPPER_HPP
#define FSTUTILS_MAPPER_HPP

/** \file
 * \brief Generalized weight mapper functor
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */

namespace fst {

/**
 * \brief templated Mapper that modifies weights over an FST, passing through the other values of the arc.
 * This mapper is to be combined with any functor that accepts Arc::Weight as a unique parameter for operator () and returns Arc::Weight.
 * The functor itself implements the mapping details, e.g. see lexicographic unit tests
 */
template<class Arc, class WeightMakerFunctorT>
class GenericWeightAutoMapper {
 public:
  ///Constructor
  explicit GenericWeightAutoMapper ( const WeightMakerFunctorT& mw )
      : mw_( mw )
  {};

  ///Takes arc as input parameter and returns modified arc
  inline Arc operator() ( const Arc& arc ) const {
    return Arc ( arc.ilabel, arc.olabel, mw_ ( arc.weight ), arc.nextstate );
  }

  inline MapSymbolsAction InputSymbolsAction() const {
    return MAP_COPY_SYMBOLS;
  }
  inline MapSymbolsAction OutputSymbolsAction() const {
    return MAP_COPY_SYMBOLS;
  }
  inline MapFinalAction FinalAction() const {
    return MAP_NO_SUPERFINAL;
  }
  inline uint Properties ( uint props ) const {
    return props;
  }

  ///Specialized functor that modifies arc weights.
  WeightMakerFunctorT const &mw_;

};

/**
 * \brief templated Mapper that modifies weights when copying from one FST to another, passing through the other values of the arc.
 * This mapper is to be combined with any functor that accepts FromArc::Weight as a unique parameter for operator () and returns ToArc::Weight.
 * The functor itself implements the mapping details.
 */

template<class FromArc, class ToArc, class WeightMakerFunctorT >
class GenericWeightMapper {
 public:
  explicit GenericWeightMapper ( const WeightMakerFunctorT& mw ) : mw_ ( mw ) {};

  ToArc operator() ( const FromArc& arc ) const {
    if ( arc.weight == FromArc::Weight::Zero() )
      return ToArc ( arc.ilabel, arc.olabel, ToArc::Weight::Zero(), arc.nextstate );
    if ( arc.weight == FromArc::Weight::One() )
      return ToArc ( arc.ilabel, arc.olabel, ToArc::Weight::One(), arc.nextstate );
    return ToArc ( arc.ilabel, arc.olabel, mw_ ( arc.weight ), arc.nextstate );
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
  uint Properties ( uint props ) const {
    return ( props & kWeightInvariantProperties ) | kUnweighted;
  }

 private:
  ///Specialized functor that modifies arc weights.
  WeightMakerFunctorT const &mw_;
};


template<class FromArc, class ToArc, class ArcMakerFunctorT >
class GenericArcMapper {
 public:
  explicit GenericArcMapper ( const ArcMakerFunctorT& ma ) : ma_ ( ma ) {};

  ToArc operator() ( const FromArc& arc ) const {
    return ma_(arc);
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
  uint Properties ( uint props ) const {
    return ( props & kWeightInvariantProperties ) | kUnweighted;
  }

 private:
  ArcMakerFunctorT const &ma_;
};


template<class Arc, class ArcMakerFunctorT >
class GenericArcAutoMapper {
 public:
  explicit GenericArcAutoMapper ( const ArcMakerFunctorT& ma ) : ma_ ( ma ) {};

  Arc operator() ( const Arc& arc ) const {
    return ma_(arc);
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
  uint Properties ( uint props ) const {
    return ( props & kWeightInvariantProperties ) | kUnweighted;
  }

 private:
  ArcMakerFunctorT const &ma_;
};



/**
 * \brief templated Mapper that inserts a word penalty over an FST, skipping user defined epsilon arcs.
 */

template<class Arc>
class WordPenaltyMapper {
 public:
  ///Constructor
  explicit WordPenaltyMapper ( typename Arc::Weight wp,
                               unordered_set<typename Arc::Label> epsilons) :
    epsilons_ (epsilons),
    wp_ ( wp ) {
  };

  ///Takes arc as input parameter and returns modified arc
  Arc operator() ( const Arc& arc ) const {
    if (epsilons_.find (arc.ilabel) != epsilons_.end() )
      return Arc ( arc.ilabel, arc.olabel, arc.weight, arc.nextstate );
    return Arc ( arc.ilabel, arc.olabel, Times (arc.weight, wp_), arc.nextstate );
  }

  inline MapSymbolsAction InputSymbolsAction() const {
    return MAP_COPY_SYMBOLS;
  }
  inline MapSymbolsAction OutputSymbolsAction() const {
    return MAP_COPY_SYMBOLS;
  }
  inline MapFinalAction FinalAction() const {
    return MAP_NO_SUPERFINAL;
  }
  inline uint Properties ( uint props ) const {
    return props;
  }

  ///Specialized functor that modifies arc weights.
  typename Arc::Weight wp_;
  unordered_set<typename Arc::Label> epsilons_;

};

/// StdArc to LexStdArc mapper.
struct StdToLexStdMapper {
  typedef StdArc FromArc;
  typedef LexStdArc ToArc;
  typedef ToArc::Weight W;

  //If i==0, copies to both weights. If i=1 or 2, only copies stdweight to first or second weight respectively, leaving a Weight::One() on the other.
  explicit StdToLexStdMapper (int i = 0) : i_ (i) {
    CHECK (!i || i == 1 || i == 2);
  }

  LexStdArc operator() (const StdArc& arc) const {
    FromArc::Weight w1 = (i_ == 1
                          || !i_) ? arc.weight.Value() : FromArc::Weight::One();
    FromArc::Weight w2 = (i_ == 2
                          || !i_) ? arc.weight.Value() : FromArc::Weight::One();
    W w (w1, w2);
    return LexStdArc (arc.ilabel, arc.olabel, w, arc.nextstate);
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
    return props;
  }
  int i_;
};

///LexStdArc to StdArc Mapper
struct LexStdToStdMapper {
  typedef LexStdArc FromArc;
  typedef StdArc ToArc;
  typedef ToArc::Weight W;

  explicit LexStdToStdMapper() : i_ (0) {}

  explicit LexStdToStdMapper (int i) : i_ (i) {
    CHECK (i == 1 || i == 2 || i == 0);
  }

  StdArc operator() (const LexStdArc& arc) const {
    W w;
    if (i_ == 0)
      w = Times ( arc.weight.Value1(), arc.weight.Value2() );
    if (i_ == 1)
      w = arc.weight.Value1();
    if (i_ == 2)
      w = arc.weight.Value2();
    return StdArc (arc.ilabel, arc.olabel, w, arc.nextstate);
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
    return props;
  }

  int i_;
};

}; //namespace

#endif
