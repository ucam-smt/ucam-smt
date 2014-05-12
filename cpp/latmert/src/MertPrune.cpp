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

#include "MertPrune.h"

PruneStats::PruneStats() :
  before (0), after (0) {
}
;

void Prune (fst::MutableFst<FunctionArc> *fst, PruneStats& stats) {
  stats.before += ArcCount (*fst);
  Prune (fst);
  stats.after += ArcCount (*fst);
}

void FullPrune (fst::MutableFst<FunctionArc> *fst,
                const std::vector<FunctionWeight>& distance, PruneStats& stats) {
  stats.before += ArcCount (*fst);
  FullPrune (fst, distance);
  stats.after += ArcCount (*fst);
}

void Prune (fst::MutableFst<FunctionArc> *fst) {
  std::map<StatePair, int> dupeArcs;
  std::map<StatePair, FunctionWeight> summedWeights;
  int totalArcs = 0;
  for (fst::StateIterator < fst::MutableFst<FunctionArc> > siter (*fst);
       !siter.Done(); siter.Next() ) {
    FunctionArc::StateId startState = siter.Value();
    int arcCount = 0;
    for (fst::ArcIterator < fst::MutableFst<FunctionArc> > aiter (*fst,
         siter.Value() ); !aiter.Done(); aiter.Next() ) {
      ++totalArcs;
      ++arcCount;
      const FunctionArc& arc = aiter.Value();
      FunctionArc::StateId endState = arc.nextstate;
      StatePair pair (startState, endState);
      if (summedWeights.count (pair) == 0) {
        summedWeights[pair] = arc.weight;
        dupeArcs[pair] = 1;
      } else {
        summedWeights[pair] = Plus (summedWeights[pair], arc.weight);
        dupeArcs[pair] += 1;
      }
    }
    //cout << startState << "\t" << arcCount << endl;
  }
  FunctionArc::StateId deadState = fst->AddState();
  for (std::map<StatePair, int>::iterator it = dupeArcs.begin(); it
       != dupeArcs.end(); ++it) {
    if (it->second > 1) {
      for (fst::MutableArcIterator < fst::MutableFst<FunctionArc> > aiter (fst,
           it->first.first); !aiter.Done(); aiter.Next() ) {
        FunctionArc arc = aiter.Value();
        bool keepArc = false;
        if (arc.nextstate == it->first.second) {
          for (MertList::const_iterator miter =
                 summedWeights[it->first].Value().begin(); miter
               != summedWeights[it->first].Value().end(); ++miter) {
            keepArc |= arc.weight.Value().front() == *miter;
          }
          if (!keepArc) {
            arc.nextstate = deadState;
            aiter.SetValue (arc);
          }
        }
      }
    }
    //cout << it->first.first << "->" << it->first.second << "\t"
    //    << it->second << endl;
  }
  Connect (fst);
}

void FullPrune (fst::MutableFst<FunctionArc> *fst,
                const std::vector<FunctionWeight>& distance) {
  FunctionArc::StateId deadState = fst->AddState();
  for (fst::StateIterator < fst::MutableFst<FunctionArc> > siter (*fst);
       !siter.Done(); siter.Next() ) {
    std::map<FunctionArc::StateId, FunctionWeight> otherArcs;
    for (fst::ArcIterator < fst::Fst<FunctionArc> > aiter (*fst, siter.Value() );
         !aiter.Done(); aiter.Next() ) {
      FunctionArc arc = aiter.Value();
      if (otherArcs.count (arc.nextstate) == 0) {
        otherArcs[arc.nextstate] = Times (arc.weight, distance[arc.nextstate]);
      } else {
        otherArcs[arc.nextstate] = Plus (otherArcs[arc.nextstate],
                                         Times (arc.weight, distance[arc.nextstate]) );
      }
    }
    /*for (map<FunctionArc::StateId, FunctionWeight>::iterator miter =
        otherArcs.begin(); miter != otherArcs.end(); ++miter) {
      miter->second = Times(miter->second, distance[miter->first]);
    }*/
    std::set<FunctionArc::StateId> toDelete;
    for (std::map<FunctionArc::StateId, FunctionWeight>::iterator outer =
           otherArcs.begin(); outer != otherArcs.end(); ++outer) {
      FunctionWeight otherSum = FunctionWeight::Zero();
      for (std::map<FunctionArc::StateId, FunctionWeight>::iterator inner =
             otherArcs.begin(); inner != otherArcs.end(); ++inner) {
        if (outer->first != inner->first) {
          otherSum = Plus (otherSum, inner->second);
        }
      }
      if (otherSum == distance[siter.Value()]) {
        toDelete.insert (outer->first);
      }
    }
    for (fst::MutableArcIterator < fst::MutableFst<FunctionArc> > aiter (fst,
         siter.Value() ); !aiter.Done(); aiter.Next() ) {
      FunctionArc arc = aiter.Value();
      if (toDelete.count (arc.nextstate) > 0) {
        arc.nextstate = deadState;
        aiter.SetValue (arc);
      }
    }
  }
  Connect (fst);
}

int ArcCount (const fst::Fst<FunctionArc>& fst) {
  int arcs = 0;
  for (fst::StateIterator < fst::Fst<FunctionArc> > siter (fst); !siter.Done();
       siter.Next() ) {
    FunctionArc::StateId startState = siter.Value();
    for (fst::ArcIterator < fst::Fst<FunctionArc> > aiter (fst, siter.Value() );
         !aiter.Done(); aiter.Next() ) {
      ++arcs;
    }
    //cout << startState << "\t" << arcCount << endl;
  }
  return arcs;
}
