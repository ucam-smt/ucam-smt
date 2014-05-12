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

#include "Score.h"

Sentence GetBestHypothesisTokens (const
                                  fst::VectorFst<fst::ArcTpl<FeatureWeight> >* fst) {
  fst::VectorFst<fst::ArcTpl<FeatureWeight> > fsttmp1;
  fst::ShortestPath (*fst, &fsttmp1);
  fst::Project (&fsttmp1, fst::PROJECT_INPUT);
  fst::RmEpsilon (&fsttmp1);
  fst::TopSort (&fsttmp1);
  Sentence best;
  for (fst::StateIterator<fst::VectorFst<fst::ArcTpl<FeatureWeight> > > si (
         fsttmp1);
       !si.Done(); si.Next() ) {
    for (fst::ArcIterator<fst::VectorFst<fst::ArcTpl<FeatureWeight> > > ai (fsttmp1,
         si.Value() ); !ai.Done(); ai.Next() ) {
      best.push_back (ai.Value().ilabel);
    }
  }
  return best;
}

Sentence GetBestHypothesis (const TupleArcFst* fst, const PARAMS& lambda) {
  fst::VectorFst<fst::ArcTpl<FeatureWeight> > * fsttmp1 = new fst::VectorFst <
  fst::ArcTpl<FeatureWeight> > ();
  fst::DotProductMap<double> m (lambda);
  Map (
    *fst,
    fsttmp1,
    fst::GeneralMapper<TupleArc, fst::ArcTpl<FeatureWeight>, fst::DotProductMap<double> >
    (
      m) );
  fst::VectorFst<fst::ArcTpl<FeatureWeight> > *fsttmp2 = new fst::VectorFst <
  fst::ArcTpl<FeatureWeight> >;
  ShortestPath (*fsttmp1, fsttmp2);
  delete fsttmp1;
  Sentence h = GetBestHypothesisTokens (fsttmp2);
  delete fsttmp2;
  int offset;
  h.size() < 2 ? offset = 0 : offset = 1;
  Sentence s = NGram (h.begin() + offset,
                      h.end() - offset); // Trim sentence start and end markers from hypothesis
  return s;
}

