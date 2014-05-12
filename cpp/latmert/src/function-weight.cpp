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

#include "function-weight.h"
#include <fst/fstlib.h>
#include <limits>
#include <ostream>
#include <iostream>
#include <algorithm>

std::ostream& operator<< (std::ostream& os, const MertLine& ml) {
  return os << "x:" << ml.x << " y:" << ml.y << " m:" << ml.m;
}

std::istream& operator>> (std::istream& is, MertLine& ml) {
  // Two chars to pull out values
  char a, b;
  is >> a >> b >> ml.x >> a >> b >> ml.y >> a >> b >> ml.m;
  return is;
}

std::ostream& operator<< (std::ostream& os, const MertList& l) {
  os << "[";
  for (std::list<MertLine>::const_iterator it = l.begin(); it != l.end(); it++) {
    os << *it << ", ";
  }
  return os << "]";
}

bool operator== (const MertLine& thisML, const MertLine& that) {
  return thisML.y == that.y && thisML.m == that.m;
}

CompositeList::CompositeList (const MertList& l1, const MertList& l2) :
  l1 (l1), l2 (l2) {
}

CompIter::cl_iterator (MertIter i1, MertIter i2, MertIter i1end,
                       MertIter i2end) :
  i1 (i1), i2 (i2), i1end (i1end), i2end (i2end) {
  if (i1 == i1end && i2 != i2end) {
    currIter = i2;
    currList = 2;
  } else if (i1 != i1end && i2 == i2end) {
    currIter = i1;
    currList = 1;
  } else if (i1 != i1end && i2 != i2end) {
    if (i1->m > i2->m) {
      currIter = i1;
      currList = 1;
    } else {
      currIter = i2;
      currList = 2;
    }
  }
}
/*
 CompIter::cl_iterator(const const_iterator& that) :
 i1(that.i1), i2(that.i2), currList(that.currList), currIter(that.currIter) {
 }

 const CompIter& CompIter::operator=(const const_iterator& that) {
 i1 = that.i1;
 i2 = that.i2;
 currList = that.currList;
 currIter = that.currIter;
 return *this;
 }
 */
MertLine& CompIter::operator*() {
  return *currIter;
}

CompIter& CompIter::operator++() {
  if (currList == 1 && i1 != i1end) {
    ++i1;
  } else if (currList == 2 && i2 != i2end) {
    ++i2;
  }
  if (i2 != i2end && i1 != i1end) {
    if (i1->m > i2->m) {
      currIter = i1;
      currList = 1;
    } else {
      currIter = i2;
      currList = 2;
    }
  }
  if (i2 == i2end && i1 != i1end) {
    currIter = i1;
    currList = 1;
  } else if (i2 != i2end && i1 == i1end) {
    currIter = i2;
    currList = 2;
  }
  return *this;
}

CompIter CompIter::operator++ (int) {
  CompIter ans = *this;
  ++ (*this);
  return ans;
}

bool CompIter::operator== (const CompIter& that) {
  if (this->isEnd() == that.isEnd() ) {
    return true;
  }
  bool equals = true;
  equals &= this->i1 == that.i1;
  equals &= this->i2 == that.i2;
  equals &= this->currList == that.currList;
  equals &= this->currIter == that.currIter;
  return equals;
}

bool CompIter::operator!= (const CompIter& that) {
  return ! (*this == that);
}

bool CompIter::isEnd() const {
  return i1 == i1end && i2 == i2end;
}

CompIter CompositeList::begin() {
  return CompIter (l1.begin(), l2.begin(), l1.end(), l2.end() );
}

CompIter CompositeList::end() {
  return CompIter (l1.end(), l2.end(), l1.end(), l2.end() );
}

bool ApproxEqual (const FunctionWeight& w1,
                  const FunctionWeight& w2, float delta) {
  return w1 == w2;
}

bool FunctionWeight::operator== (const FunctionWeight& w1) const {
  if (this->values.size() != w1.values.size() ) {
    return false;
  }
  return equal (this->values.begin(), this->values.end(), w1.values.begin() );
}

bool FunctionWeight::operator!= (const FunctionWeight& w1) const {
  return ! (*this == w1);
}

fst::TropicalWeightTpl<F> FunctionWeight::Map (F gamma) const {
  //Assuming that we're a monomial weight
  MertLine l = values.front();
  return fst::TropicalWeightTpl<F> (l.m * gamma + l.y);
}

FunctionWeight Times (const FunctionWeight& w1,
                      const FunctionWeight& w2) {
  if (w1 == FunctionWeight::Zero() || w2 == FunctionWeight::Zero() ) {
    return FunctionWeight::Zero();
  };
  MertList singleton;
  MertList fullList;
  if (w1.values.size() == 1) {
    singleton = w1.values;
    fullList = w2.values;
  } else if (w2.values.size() == 1) {
    singleton = w2.values;
    fullList = w1.values;
  } else {
    cout << "No singleton list!" << endl;
    exit (1);
  }
  MertLine multiplier = singleton.front();
  MertList output;
  /*  if(multiplier == zeroLine)
   {
   output.push_back(zeroLine);
   return output;
   }*/
  for (MertIter it = fullList.begin(); it != fullList.end(); ++it) {
    MertLine l (*it);
    l.y += multiplier.y;
    l.m += multiplier.m;
    //l.t.push_back(multiplier.t.front());
    output.push_back (l);
  }
  return FunctionWeight (output);
}

FunctionWeight Plus (const FunctionWeight& w1,
                     const FunctionWeight& w2) {
  MertList outputList;
  if (w1 == FunctionWeight::Zero() ) {
    outputList.insert (outputList.begin(), w2.values.begin(), w2.values.end() );
    return FunctionWeight (outputList);
  } else if (w2 == FunctionWeight::Zero() ) {
    outputList.insert (outputList.begin(), w1.values.begin(), w1.values.end() );
    return FunctionWeight (outputList);
  }
  CompositeList lines (w1.values, w2.values);
  // Use a dummy variable to represent the start of the list
  static const MertLine dummy (0, 0, 0);
  outputList.push_front (dummy);
  MertList::iterator output = outputList.begin();
  for (CompIter l = lines.begin(); l != lines.end(); ++l) {
    if (output != outputList.begin() ) {
      if (output->m == (*l).m) {
        if ( (*l).y >= output->y)
          continue;
        --output;
      }
      while (output != outputList.begin() ) {
        (*l).x = ( (*l).y - output->y) / (output->m - (*l).m);
        if (output->x < (*l).x)
          break;
        --output;
      }
      if (output == outputList.begin() )
        (*l).x = -std::numeric_limits<double>::infinity();
      output = outputList.insert (++output, *l);
    } else {
      output = outputList.insert (++output, *l);
    }
  }
  //Pop the dummy variable off
  outputList.pop_front();
  //Erase any non-contributing lines off the list
  outputList.erase (++output, outputList.end() );
  FunctionWeight out (outputList);
  return out;
}

std::ostream& operator<< (std::ostream& strm, const FunctionWeight& w) {
  const MertList v = w.Value();
  strm << v.size();
  for (MertList::const_iterator it = v.begin(); it != v.end(); ++it) {
    MertLine l = *it;
    strm << l.y << l.m;
  }
  return strm;
}

std::istream& operator>> (std::istream& strm, FunctionWeight& w) {
  MertList::size_type size;
  strm >> size;
  MertList outList;
  for (MertList::size_type i = 0; i < size; ++i) {
    MertLine l;
    strm >> l.y >> l.m;
    outList.push_back (l);
  }
  FunctionWeight tw (outList);
  w.values = outList;
  return strm;
}

std::ostream& FunctionWeight::Write (std::ostream& strm) const {
  const MertList v = this->Value();
  strm << v.size();
  for (MertList::const_iterator it = v.begin(); it != v.end(); ++it) {
    MertLine l = *it;
    strm << l.y << l.m;
  }
  return strm;
}

std::istream& FunctionWeight::Read (std::istream& strm) {
  MertList::size_type size;
  strm >> size;
  MertList outList;
  for (MertList::size_type i = 0; i < size; ++i) {
    MertLine l;
    strm >> l.y >> l.m;
    outList.push_back (l);
  }
  this->values = outList;
  return strm;
}

std::size_t FunctionWeight::Hash() const {
  cout << "aaw35 Tried to access hash function" << endl;
  exit (1);
}

bool FunctionWeight::Member() const {
  return false;
}

FunctionWeight FunctionWeight::Divide (const FunctionWeight& fw1,
                                       const FunctionWeight&, fst::DivideType) {
  cout << "aaw35 Tried to access divide function" << endl;
  exit (1);
}

FunctionWeight::ReverseWeight FunctionWeight::Reverse() const {
  return *this;
}

