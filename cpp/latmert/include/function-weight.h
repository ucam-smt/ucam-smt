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

#ifndef FUNCTION_WEIGHT_H_
#define FUNCTION_WEIGHT_H_

#include <fst/fstlib.h>
#include <list>
#include <vector>
#include <limits>
#include <iterator>
#include <ostream>

typedef long long Wid;

typedef std::vector<Wid> SentenceIdx;

typedef double F;

class MertLine {
 public:
  MertLine() :
    x (-std::numeric_limits<double>::infinity() ), y (0.0), m (0.0) {
  }

  MertLine (double y, double m, Wid word) :
    x (-std::numeric_limits<double>::infinity() ), y (y), m (m) {
    //t.push_back(word);
  }
  double x; // x-intercept of left-adjacent line
  double y; // y-intercept of line
  double m; // slope of line
  SentenceIdx t; // partial translation hypothesis associated with line
  double score; //translation score
};

typedef std::list<MertLine> MertList;
typedef MertList::iterator MertIter;

const MertLine zeroLine (std::numeric_limits<double>::infinity(), 0, 0);
const MertLine oneLine (0, 0, 0);

static MertList InitFW (MertLine l) {
  MertList ml;
  ml.push_back (l);
  return ml;
}

const MertList zeroList = InitFW (zeroLine);
const MertList oneList = InitFW (oneLine);

std::ostream& operator<< (std::ostream&, const MertLine&);

std::ostream& operator<< (std::ostream&, const MertList&);

std::istream& operator>> (std::istream&, MertList&);

bool operator== (const MertLine&, const MertLine&);

/*
 * Holds two sorted lists and returns the correct value
 */
class CompositeList {

 public:
  CompositeList (const MertList& l1, const MertList& l2);

  class cl_iterator: std::iterator<std::input_iterator_tag, MertLine> {
   public:
    cl_iterator (MertIter, MertIter, MertIter, MertIter);
    MertLine& operator*();
    cl_iterator& operator++();
    cl_iterator operator++ (int);
    bool operator== (const cl_iterator&);
    bool operator!= (const cl_iterator&);

   private:
    MertIter i1;
    MertIter i2;
    MertIter i1end;
    MertIter i2end;
    int currList;
    MertIter currIter;
    bool isEnd() const;
  };

  cl_iterator begin();
  cl_iterator end();

 private:
  MertList l1;
  MertList l2;
};

typedef CompositeList::cl_iterator CompIter;

class FunctionWeight {
 public:

  typedef FunctionWeight ReverseWeight;

  static const FunctionWeight& Zero() {
    static FunctionWeight zero (zeroList);
    return zero;
  }

  static const FunctionWeight& One() {
    static FunctionWeight one (oneList);
    return one;
  }

  FunctionWeight() :
    values (FunctionWeight::Zero().values) {
  }

  FunctionWeight (MertList& values) :
    values (values) {
  }

  FunctionWeight (const MertList& values) {
    this->values = values;
  }

  const MertList& Value() const {
    return values;
  }

  bool operator== (const FunctionWeight&) const;

  bool operator!= (const FunctionWeight&) const;

  FunctionWeight Divide (const FunctionWeight& fw1, const FunctionWeight&,
                         fst::DivideType t = fst::DIVIDE_ANY);

  std::size_t Hash() const;

  bool Member() const;

  ReverseWeight Reverse() const;

  std::istream& Read (std::istream& strm);

  std::ostream& Write (std::ostream& strm) const;

  fst::TropicalWeightTpl<F> Map (F) const;

  static uint64 Properties() {
    return fst::kLeftSemiring | fst::kRightSemiring | fst::kCommutative |
           fst::kIdempotent;
  }
  ;

  friend std::istream& operator>> (std::istream& strm, FunctionWeight& w);

  friend FunctionWeight Plus (const FunctionWeight&, const FunctionWeight&);

  friend FunctionWeight Times (const FunctionWeight&, const FunctionWeight&);

  static const std::string& Type() {
    static const std::string type = "Function";
    return type;
  }

 private:
  MertList values;

};

bool ApproxEqual (const FunctionWeight&, const FunctionWeight&, float);

FunctionWeight Plus (const FunctionWeight&, const FunctionWeight&);

FunctionWeight Times (const FunctionWeight&, const FunctionWeight&);

std::ostream& operator<< (std::ostream& strm, const FunctionWeight& w);

fst::TropicalWeightTpl<F> Map (double);

struct FunctionArc {
  typedef int Label;
  typedef FunctionWeight Weight;
  typedef int StateId;

  // Vector arc initialised from a function weight
  FunctionArc (Label i, Label o, Weight w, StateId s) :
    ilabel (i), olabel (o), weight (w), nextstate (s) {
  }

  FunctionArc() {
  }

  static const std::string& Type() {
    static const std::string type = "Function";
    return type;
  }

  Label ilabel; // Transition input label
  Label olabel; // Transition output label
  Weight weight; // Transition weight
  StateId nextstate; // Transition destination state
};

template<class W, unsigned int n>
F DotProduct (const W&, const fst::TupleWeight<fst::TropicalWeightTpl<F>, n>&);

template<unsigned int n>
F DotProduct (const fst::TupleWeight<fst::TropicalWeightTpl<F>, n>& features
              , const std::vector<F>& params) {
  F result = 0.0;
  for (unsigned int i = 0; i < features.Length(); ++i) {
    result += features.Value (i).Value() * params[i];
  }
  return result;
}

template<class FromArc, class T>
class VectorToFunctionMapper {

 public:
  typedef FunctionArc ToArc;
  typedef typename FromArc::Weight FW; // From weight (vector)
  typedef ToArc::Weight TW; // To weight (Function)
  explicit VectorToFunctionMapper (const std::vector<T>& direction,
                                   const std::vector<T>& initial) :

    direction (direction), initial (initial) {
  }
  ToArc operator() (const FromArc& arc) const {
    //FW features = arc.weight.Value();
    FW features = arc.weight;
    if (features == FW::Zero() ) {
      return ToArc (arc.ilabel, arc.olabel, TW::Zero(), arc.nextstate);
    }
    if (features == FW::One() ) {
      return ToArc (arc.ilabel, arc.olabel, TW::One(), arc.nextstate);
    }
    F m = DotProduct (features, direction);
    F y = DotProduct (features, initial);
    MertList mlist;
    mlist.push_back (MertLine (y, m, arc.ilabel) );
    //    cout << mlist << endl;
    TW mapped (mlist);
    return ToArc (arc.ilabel, arc.olabel, mapped, arc.nextstate);
  }
  fst::MapSymbolsAction InputSymbolsAction() const {
    return fst::MAP_COPY_SYMBOLS;
  }
  fst::MapSymbolsAction OutputSymbolsAction() const {
    return fst::MAP_COPY_SYMBOLS;
  }
  fst::MapFinalAction FinalAction() const {
    return fst::MAP_NO_SUPERFINAL;
  }
  uint Properties (uint props) const {
    return props;
  }

 private:

  const std::vector<T>& direction;

  const std::vector<T>& initial;

};

class FunctionToStdMapper {

 public:
  typedef FunctionArc FromArc;
  typedef fst::ArcTpl<fst::TropicalWeightTpl<double> > ToArc;
  typedef FromArc::Weight FW; // From weight (Function)
  typedef ToArc::Weight TW; // To weight (Std)
  explicit FunctionToStdMapper (double gamma) :
    gamma (gamma) {
  }
  ToArc operator() (const FromArc& arc) const {
    const MertList& function = arc.weight.Value();
    if (arc.weight.Value() == FW::Zero().Value() ) {
      return ToArc (arc.ilabel, arc.olabel, TW::Zero(), arc.nextstate);
    }
    if (arc.weight.Value() == FW::One().Value() ) {
      return ToArc (arc.ilabel, arc.olabel, TW::One(), arc.nextstate);
    }
    if (function.size() != 1) {
      cout << "Function arc has more than one function";
      exit (1);
    }
    TW mapped (function.front().m * gamma + function.front().y);
    return ToArc (arc.ilabel, arc.olabel, mapped, arc.nextstate);
  }
  fst::MapSymbolsAction InputSymbolsAction() const {
    return fst::MAP_COPY_SYMBOLS;
  }
  fst::MapSymbolsAction OutputSymbolsAction() const {
    return fst::MAP_COPY_SYMBOLS;
  }
  fst::MapFinalAction FinalAction() const {
    return fst::MAP_NO_SUPERFINAL;
  }
  uint Properties (uint props) const {
    return props;
  }

 private:

  double gamma;

};

#endif /* FUNCTION_WEIGHT_H_ */
