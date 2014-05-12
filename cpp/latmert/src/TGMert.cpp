//Copyright (c) 2011, University of Cambridge
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met://
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of Cambridge nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "TGMert.h"
#include "function-weight.h"
#include "fast-shortest-distance.h"
#include "mapping-shortest-path.h"

void const ComputeShortestPath (const fst::VectorFst<FunctionArc>& fst,
                                double x, MertLine& line) {
  fst::VectorFst < fst::ArcTpl<fst::TropicalWeightTpl<double> > > pathFst;
  mertfst::SingleShortestPath (fst, &pathFst, x);
  TopSort (&pathFst);
  for (fst::StateIterator
       < fst::VectorFst<fst::ArcTpl<fst::TropicalWeightTpl<double> > >
       > siter (pathFst); !siter.Done(); siter.Next() ) {
    for (fst::ArcIterator
         < fst::VectorFst<fst::ArcTpl<fst::TropicalWeightTpl<double> > >
         > aiter (pathFst, siter.Value() ); !aiter.Done(); aiter.Next() ) {
      line.t.push_back (aiter.Value().olabel);
      line.score += aiter.Value().weight.Value();
    }
  }
}

MertList ComputeFromFunctionArc (const fst::VectorFst<FunctionArc>& fst) {
  FunctionWeight sd = mertfst::ShortestDistance (fst);
  // Need to handle first case.
  const MertList& mlconst = sd.Value();
  // Hacky but am really getting fed up with all constness
  // and don't want to keep on copying this list
  MertList& ml = const_cast<MertList&> (mlconst);
  if (ml.size() == 1) {
    ComputeShortestPath (fst, 0, ml.front() );
  } else {
    double x = (++ml.begin() )->x;
    ComputeShortestPath (fst, x - 1, ml.front() );
    MertIter prev = ++ml.begin();
    for (MertIter next =
           ++ (++ml.begin() ); next != sd.Value().end(); ++next) {
      // Sometimes can end up with a very large gamma interval in the region of 1e16. When this happens the
      // midpoint is also huge. Any shortest path computations get corrupted for that interval. Therefore use this
      // little hack to protect ourselves from it. The value of 1000 is a magic number and has no significance
      // other than it empirically seems OK.
      double midpoint;
      if ( (next->x - prev->x) / 2 > 1000) {
        if (abs (prev->x) < abs (next->x) ) {
          midpoint = prev->x + 1000;
        } else {
          midpoint = next->x - 1000;
        }
      } else {
        //Do normal midpoint calc
        midpoint = (prev->x + next->x) / 2.0;
      }
      ComputeShortestPath (fst, midpoint, *prev);
      ++prev;
    }
    ComputeShortestPath (fst, prev->x + 1, ml.back() );
  }
  return ml;
}
