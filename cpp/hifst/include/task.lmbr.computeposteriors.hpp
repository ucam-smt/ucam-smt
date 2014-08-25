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

// Copyright 2012 - Gonzalo Iglesias, Graeme Blackwood, Adrià de Gispert, William Byrne

#ifndef TASK_LMBR_COMPUTEPOSTERIORS_HPP
#define TASK_LMBR_COMPUTEPOSTERIORS_HPP

/**
 * \file
 * \brief Based on Graeme Blackwood's PhD work and original code -- implementation of posterior computation from evidence space
 * \date 10-01-2013
 * \author Gonzalo Iglesias, Graeme Blackwood
 */

namespace ucam {
namespace lmbr {

//Functor that computes posteriors over an evidence space
class ComputePosteriors {
  //Private variables
 private:
  typedef fst::NGramList NGramList;
  typedef fst::NGramVector NGramVector;
  typedef fst::WordId WordId;

  NGramToStateMapper statemapper;
  const std::vector<NGramList>& ngrams;
  NGramToPosteriorsMapper posteriors;
  uint minorder_, maxorder_;

  //Public methods...
 public:
  ///Initialize from list of ngrams
  ComputePosteriors (const std::vector<NGramList>& ng) : minorder_ (1),
    maxorder_ (4),
    ngrams (ng) {};

  ///Set order
  inline void setOrders (uint minorder, uint maxorder) {
    minorder_ = minorder;
    maxorder_ = maxorder;
  };

  ///Compute posteriors
  inline void operator() (const fst::VectorFst<fst::StdArc>* fstlat) {
    fastComputePosteriors (fstlat);
    statemapper.clear();
  };

  ///Retrieve reference to posteriors
  inline NGramToPosteriorsMapper& getPosteriors() {
    return posteriors;
  };

  ///Write posteriors to file
  inline void WritePosteriors (const std::string& filename) {
    LINFO ("writing posterior probabilities: " << filename);
    ucam::util::oszfstream ofs (filename.data() );
    WritePosteriors (ofs);
    ofs.close();
  };

  ///Write posteriors to a szfstream
  inline void WritePosteriors ( ucam::util::oszfstream& ofs) {
    for (uint n = minorder_; n <= maxorder_; ++n) {
      for (NGramList::const_iterator it = ngrams[n].begin(); it != ngrams[n].end();
           ++it) {
        fst::NGram w = it->first;
        ofs << std::fixed << std::setprecision (12) << posteriors[w][0][0];
				using ucam::util::operator<<;
				ofs << "\t" << w << '\n';
      }
    }
  };

  //Private methods
 private:

  ///Given history, retrieve corresponding state
  fst::StdArc::StateId GetState (const fst::NGram& w) {
    NGramToStateMapper::iterator it = statemapper.find (w);
    if (it != statemapper.end() ) {
      return it->second;
    }
    return -1;
  };

  ///Push in Log semiring, set final weights to 1
  fst::VectorFst<fst::StdArc>* ConvertToPosteriors (const
      fst::VectorFst<fst::StdArc>* fst) {
    fst::VectorFst<fst::StdArc> tmp1 (*fst);
    fst::RmEpsilon (&tmp1);
    fst::VectorFst<fst::StdArc> tmp2;
    fst::Determinize (tmp1, &tmp2);
    fst::Minimize (&tmp2);
    fst::VectorFst<fst::LogArc>* tmp3 = StdToLog (&tmp2);
    fst::VectorFst<fst::LogArc> tmp4;
    fst::Push<fst::LogArc, fst::REWEIGHT_TO_FINAL> (*tmp3, &tmp4,
        fst::kPushWeights);
    delete tmp3;
    fst::VectorFst<fst::StdArc>* tmp5 = LogToStd (&tmp4);
    SetFinalStateCost (tmp5, fst::StdArc::Weight::One() );
    fst::VectorFst<fst::StdArc>* fstret = new fst::VectorFst<fst::StdArc> (*tmp5);
    delete tmp5;
    return fstret;
  };

  ///Generate list of posteriors from evidence space fstlat.
  void fastComputePosteriors (const fst::VectorFst<fst::StdArc>* fstlat) {
    LINFO ("computing posteriors: fast mode (vector-based)" );
    fst::VectorFst<fst::StdArc>* fsttmp = ConvertToPosteriors (fstlat);
    initializeStateMap();
    initializePosteriorsTable();
    for (uint n = minorder_; n <= maxorder_; ++n) {
      FastComputePosteriorsVectorForward (fsttmp, n);
    }
    delete fsttmp;
  }

  ///Initialize state hash
  inline void initializeStateMap() {
    statemapper.clear();
    fst::NGram n;
    statemapper[n] = 0;
  }

  void initializePosteriorsTable() {
    for (uint n = minorder_; n <= maxorder_; ++n) {
      for (NGramList::const_iterator it = ngrams[n].begin(); it != ngrams[n].end();
           ++it) {
        posteriors[it->first].resize (1);
        posteriors[it->first][0].resize (1);
      }
    }
  }

  ///Wrapper around posterior computing forward method
  void FastComputePosteriorsVectorForward (const fst::VectorFst<fst::StdArc>*
      fstlat, const uint n) {
    LINFO ("fast compute posteriors: order=" << n );
    //  NGramVector ngs(1, 0);
    NGramVector ngs (1);
    for (NGramList::const_iterator it = ngrams[n].begin(); it != ngrams[n].end();
         ++it) {
      ngs.push_back (it->first);
    }
    if (ngs.size() == 1) {
      return;
    }
    fst::LogArc::Label kSpecialLabel = ngs.size();
    fst::VectorFst<fst::LogArc>* fsttmp1 = StdToLog (fstlat);
    LINFO ("map lattice to order " << n );
    fst::VectorFst<fst::LogArc>* fsttmp2 = MapToHigherOrderLattice (fsttmp1, ngs,
                                           n);
    delete fsttmp1;
    LINFO ("Number of states=" << fsttmp2->NumStates() );
    ForwardComputePosteriors (fsttmp2, ngs);
    delete fsttmp2;
  }

  ///Key method to posterior computing. Lattice is traversed in a forward procedure and ngram list is updated with path log weights. Once you reach final states you have calculated the posterior weight.
  ///Note that ngrams of higher orders than 1 have been encoded as unigrams.
  void ForwardComputePosteriors (fst::VectorFst<fst::LogArc>* fst,
                                 const NGramVector& ngs) {
    typedef std::map<fst::LogArc::Label, fst::LogArc::Weight> LabelToWeightMapper;
    std::vector<fst::LogArc::Weight> fwdAlpha;
    std::vector<LabelToWeightMapper> ngmAlpha;
    LabelToWeightMapper ngmAlphaFinal;
    TopSort (fst);
    ngmAlpha.resize (fst->NumStates() );
    fwdAlpha.resize (fst->NumStates(), fst::LogArc::Weight::Zero() );
    fwdAlpha[fst->Start()] = fst::LogArc::Weight::One();
    for (fst::StateIterator< fst::VectorFst<fst::LogArc> > si (*fst); !si.Done();
         si.Next() ) {
      fst::LogArc::StateId q = si.Value();
      for (fst::ArcIterator< fst::VectorFst<fst::LogArc> > ai (*fst, q); !ai.Done();
           ai.Next() ) {
        fst::LogArc a = ai.Value();
        fwdAlpha[a.nextstate] = Plus (fwdAlpha[a.nextstate], Times (fwdAlpha[q],
                                      a.weight) );
        if (ngmAlpha[a.nextstate].find (a.ilabel) == ngmAlpha[a.nextstate].end() ) {
          ngmAlpha[a.nextstate][a.ilabel] = fst::LogArc::Weight::Zero();
        }
        ngmAlpha[a.nextstate][a.ilabel] = Plus (ngmAlpha[a.nextstate][a.ilabel],
                                                Times (fwdAlpha[q], a.weight) );
        for (LabelToWeightMapper::const_iterator it = ngmAlpha[q].begin();
             it != ngmAlpha[q].end(); ++it) {
          if (it->first != a.ilabel) {
            if (ngmAlpha[a.nextstate].find (it->first) == ngmAlpha[a.nextstate].end() ) {
              ngmAlpha[a.nextstate][it->first] = fst::LogArc::Weight::Zero();
            }
            ngmAlpha[a.nextstate][it->first] = Plus (ngmAlpha[a.nextstate][it->first],
                                               Times (it->second, a.weight) );
          }
        }
      }
      if (fst->Final (q) != fst::LogArc::Weight::Zero() ) {
        for (LabelToWeightMapper::const_iterator it = ngmAlpha[q].begin();
             it != ngmAlpha[q].end(); ++it) {
          if (ngmAlphaFinal.find (it->first) == ngmAlphaFinal.end() ) {
            ngmAlphaFinal[it->first] = fst::LogArc::Weight::Zero();
          }
          ngmAlphaFinal[it->first] = Plus (ngmAlphaFinal[it->first],
                                           Times (ngmAlpha[q][it->first], fst->Final (q) ) );
        }
      }
      ngmAlpha[q].clear();
    }
    for (LabelToWeightMapper::const_iterator it = ngmAlphaFinal.begin();
         it != ngmAlphaFinal.end(); ++it) {
      posteriors[ngs[it->first]][0][0] = exp (-1.0f * it->second.Value() );
    }
  }

  ///Maps to an equivalent lattice of order n in which each arc accepts an ngram of order n, rather than one single word ( order 1).
  template <class Arc>
  fst::VectorFst<Arc>* MapToHigherOrderLattice (const fst::VectorFst<Arc>* fstlat,
      const NGramVector& ngs, const uint order) {
    fst::VectorFst<Arc>* fsttmp1 = new fst::VectorFst<Arc> (*fstlat);
    fst::VectorFst<Arc>* fsttmp2 = MakeOrderMappingTransducer<Arc> (ngs, order);
    fst::VectorFst<Arc>* fsttmp3 = new fst::VectorFst<Arc>;
    Compose (*fsttmp1, *fsttmp2, fsttmp3);
    delete fsttmp1;
    delete fsttmp2;
    fst::Project (fsttmp3, fst::PROJECT_OUTPUT);
    fst::RmEpsilon (fsttmp3);
    fst::VectorFst<Arc>* fsttmp4 = new fst::VectorFst<Arc>;
    fst::Determinize (*fsttmp3, fsttmp4);
    delete fsttmp3;
    fst::Minimize (fsttmp4);
    return fsttmp4;
  }

  ///For each ngram, Creates all states and stores ids in a hash using ngram as key
  template <class Arc>
  void MakeOrderMappingTransducerHistory (fst::VectorFst<Arc>* fst,
                                          const fst::NGram& h) {
    typedef typename Arc::StateId StateId;
    typedef typename Arc::Weight Weight;
    StateId src = fst->Start();
    StateId trg = fst->Start();
    for (uint i = 0; i < h.size(); ++i) {
      trg = fst->AddState();
      fst->AddArc (src, Arc (h[i], kEpsLabel, Weight::One(), trg) );
      src = trg;
      if (i + 1 == h.size() ) {
        fst->SetFinal (trg, Weight::One() );
      }
    }
    statemapper[h] = trg;
  }

  ///Creates a transducer of order n. Once generated, you can compose with a word of lattice to generate equivalent higher order lattice accepting ngrams of order n instead of words.
  template <class Arc>
  fst::VectorFst<Arc>* MakeOrderMappingTransducer (const NGramVector& ngs,
      const uint order) {
    typedef typename Arc::StateId StateId;
    typedef typename Arc::Weight Weight;
    fst::VectorFst<Arc>* fst = new fst::VectorFst<Arc>;
    StateId start = fst->AddState();
    fst->SetStart (start);
    fst::NGram w;
    statemapper[w] = start;
    for (uint i = 1; i < ngs.size(); ++i) {
      fst::NGram h = ngs[i];
      //    h.pop_back();
      h.resize (h.size() - 1);
      if (GetState (h) == -1) {
        MakeOrderMappingTransducerHistory (fst, h);
      }
    }
    for (uint i = 1; i < ngs.size(); ++i) {
      fst::NGram w = ngs[i];
      fst::NGram h (w.begin(),   w.end() - 1); // NGram history at source
      fst::NGram t (w.begin() + 1, w.end()  ); // NGram history at target
      StateId src = GetState (h);
      StateId trg = GetState (t);
      if (trg == -1) {
        trg = fst->AddState();
        statemapper[t] = trg;
      }
      //    WordId wid = w.back();
      WordId wid = w[w.size() - 1];
      fst->AddArc (src, Arc (wid, i, Arc::Weight::One(), trg) );
      fst->SetFinal (trg, Arc::Weight::One() );
    }
    fst::ArcSort (fst, fst::ILabelCompare<Arc>() );
    return fst;
  }

};

}
} // end namespaces

#endif
