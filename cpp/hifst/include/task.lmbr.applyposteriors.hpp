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

#ifndef TASK_LMBR_APPLYPOSTERIORS
#define TASK_LMBR_APPLYPOSTERIORS

/**
 * \file
 * \brief Based on Graeme Blackwood's PhD work and original code -- implementation of posterior application to a hypotheses space
 * \date 10-01-2013
 * \author Gonzalo Iglesias, Graeme Blackwood
 */

namespace ucam {
namespace lmbr {

///Functor handling LMBR theta parameters
class Theta {
 private:
  std::vector<fst::StdArc::Weight> theta_;
 public:

  //Constructor with no p,r,T defined -- ngram factors disabled
  Theta (unsigned minorder = 1, unsigned maxorder = 4) {
    theta_.resize (maxorder + 1);
    theta_[0] = 0.0f;
    for (int n = minorder; n <= maxorder; n++) {
      theta_[n] = 1.0f;
    }
    LINFO ( "ngram factors disabled");
    LINFO ( "theta[0]=" << std::fixed << std::setprecision (10) << theta_[0] );
    for (int n = minorder; n <= maxorder; n++) {
      LINFO ( "theta[" << n << "]=" << std::fixed << std::setprecision (
                10) << theta_[n] );
    }
  }

  ///Constructor -- Theta parameters defined from p,R,T,minorder, maxorder
  Theta (float p, float r, float T, unsigned minorder = 1,
         unsigned maxorder = 4) {
    theta_.resize (maxorder + 1);
    if (T != 0 && p != 0 &&  r != 0) {
      LINFO ( "T=" << T );
      LINFO ( "p=" << std::fixed << std::setprecision (4) << p );
      LINFO ( "r=" << std::fixed << std::setprecision (4) << r );
      theta_[0] = -1 / T;
      for (int n = minorder; n <= maxorder; n++) {
        theta_[n] = 1 / (4 * T * p * pow (r, n - 1) );
      }
    } else {
      theta_[0] = 0.0f;
      for (int n = minorder; n <= maxorder; n++) {
        theta_[n] = 1.0f;
      }
    }
    LINFO ( "theta[0]=" << std::fixed << std::setprecision (10) << theta_[0] );
    for (int n = minorder; n <= maxorder; n++) {
      LINFO ( "theta[" << n << "]=" << std::fixed << std::setprecision (
                10) << theta_[n] );
    }
  };
  inline fst::StdArc::Weight const& operator() (unsigned k) {
    return theta_[k];
  }

};

///Functor that applies posteriors to any hypothesis space. Initializes with previously calculated posteriors
class ApplyPosteriors {

 private:
  typedef fst::NGramList NGramList;
  typedef fst::WordId WordId;

  NGramToStateMapper statemapper;
  Wlist vocab;
  std::vector<NGramList>& hs_ngrams_;
  Theta& theta_;
  NGramToPosteriorsMapper& posteriors_;

  float wps_;
  //Lmbr output...
  fst::VectorFst<fst::StdArc> lmbroutput_;
  unsigned minorder_;
  unsigned maxorder_;

  //Public methods
 public:
  /**
   * \brief Constructor: initializes functor with theta, min/max order, the ngrams and the posteriors.
   */
  ApplyPosteriors ( std::vector<NGramList>& ng,
                    NGramToPosteriorsMapper& pst,
                    Theta& theta,
                    unsigned minorder = 1,
                    unsigned maxorder = 4) :
    minorder_ (minorder),
    maxorder_ (maxorder),
    posteriors_ (pst),
    hs_ngrams_ (ng),
    theta_ (theta) {
  };

  ///Applies posteriors (Graeme's fast method, four compositions). Hypotheses space previously tuned with word penalty (theta_[0]).
  fst::VectorFst<fst::StdArc> *operator() (fst::VectorFst<fst::StdArc> const
      &fsthyp) {
    LINFO ("decoding...");
    fst::VectorFst<fst::StdArc> aux;
    fst::Map (fsthyp, &aux, fst::TimesMapper<fst::StdArc> (theta_ (0) ) );
    SetFinalStateCost (&aux, fst::StdArc::Weight::One() );
    fst::VectorFst<fst::StdArc>* fstmbr = NULL;
    LINFO ("NS=" << aux.NumStates() );
    fstmbr = fastApplyPosteriors (aux);
    fst::VectorFst<fst::StdArc> *fstmax = FstScaleWeights (fstmbr, -1);
    delete fstmbr;
    return fstmax;
  };

  //Private methods
 private:
  ///Get State given an ngram.
  fst::StdArc::StateId GetState (const fst::NGram& w) {
    NGramToStateMapper::iterator it = statemapper.find (w);
    if (it != statemapper.end() ) {
      return it->second;
    }
    return -1;
  };

  ///Initialize statemapper.
  void initializeStateMap() {
    fst::NGram w;
    statemapper.clear();
    statemapper[w] = 0;
  }

  ///Compose hypothesis space with posteriors of order n.
  void applyPosteriorsEx (fst::MutableFst<fst::StdArc>* fstlat,
                          const unsigned n) {
    fst::MutableFst<fst::StdArc>* fsttmp = fstlat->Copy();
    fst::VectorFst<fst::StdArc>* fstpst = makePosteriorsFST (n);
    fst::ArcSort (fstpst, fst::ILabelCompare<fst::StdArc>() );
    fst::Compose (*fsttmp, *fstpst, fstlat);
    delete fsttmp;
    delete fstpst;
  }

  ///Apply all posteriors (mininum order to maximum order)
  fst::VectorFst<fst::StdArc>* fastApplyPosteriors (const
      fst::VectorFst<fst::StdArc>& fsthyp) {
    LINFO ("fast decoding enabled");
    initializeStateMap();
    fst::VectorFst<fst::StdArc>* fsttmp = fsthyp.Copy();
    for (unsigned n = maxorder_; n >= minorder_; --n) {
      if (hs_ngrams_[n].size() > 0) {
        applyPosteriorsEx (fsttmp, n);
      }
    }
    return fsttmp;
  }

  ///Build Unigram posterior lattice
  fst::VectorFst<fst::StdArc>* makeUnigramPosteriorsFST() {
    fst::VectorFst<fst::StdArc>* fst = new fst::VectorFst<fst::StdArc>;
    fst::StdArc::StateId startState = fst->AddState();
    fst->SetStart (startState);
    for (NGramList::const_iterator it = hs_ngrams_[1].begin();
         it != hs_ngrams_[1].end(); ++it) {
      fst::NGram w = it->first;
      fst::StdArc::Weight p = 0;
      if (posteriors_.find (w) != posteriors_.end() ) p = posteriors_[w][0][0] *
            theta_
            (1).Value();
      fst->AddArc (startState, fst::StdArc (w[0], w[0], p, startState) );
    }
    fst->SetFinal (startState, fst::StdArc::Weight::One() );
    LINFO (std::setw (6) << hs_ngrams_[1].size() << " 1-ngram gain(s) applied");
    return fst;
  }

  ///Make posterior FST of any order n
  fst::VectorFst<fst::StdArc>* makePosteriorsFST (const unsigned n) {
    if (n == 1) {
      return makeUnigramPosteriorsFST();
    }
    fst::VectorFst<fst::StdArc>* fst = new fst::VectorFst<fst::StdArc>;
    fst::StdArc::StateId startState = fst->AddState();
    fst->SetStart (startState);
    for (NGramList::const_iterator it = hs_ngrams_[n].begin();
         it != hs_ngrams_[n].end(); ++it) {
      fst::NGram h = it->first;
      //      h.pop_back();
      h.resize (h.size() - 1);
      if (GetState (h) == -1) {
        MakeHistory (fst, h);
      }
    }
    for (NGramList::const_iterator it = hs_ngrams_[n].begin();
         it != hs_ngrams_[n].end(); ++it) {
      fst::NGram w = it->first;
      fst::NGram h, t;
      h.assign ( w.begin(), w.end() - 1);
      t.assign ( w.begin() + 1, w.end() );
      fst::StdArc::StateId src = GetState (h);
      fst::StdArc::StateId trg = GetState (t);
      if (trg == -1) {
        trg = fst->AddState();
        fst->SetFinal (trg,
                       fst::StdArc::Weight::One() ); //Gonzalo: Set all states to final.
        statemapper[t] = trg;
      }
      WordId wid = t[t.size() - 1];
      fst::StdArc::Weight p = 0;
      if (posteriors_.find (w) != posteriors_.end() ) p = posteriors_[w][0][0] *
            theta_
            (n).Value();
      fst->AddArc (src, fst::StdArc (wid, wid, p, trg) );
      fst->SetFinal (trg, fst::StdArc::Weight::One() );
    }
    LINFO (std::setw (6) << hs_ngrams_[n].size() << " " << n <<
           "-ngram gain(s) applied");
    return fst;
  }

  ///Creates a state and writes down in a hash variable the corresponding history.
  void MakeHistory (fst::VectorFst<fst::StdArc>* fst, const fst::NGram& h) {
    fst::StdArc::StateId src = fst->Start();
    fst::StdArc::StateId trg;
    for (fst::NGram::const_iterator it = h.begin(); it != h.end(); ++it) {
      WordId wid = *it;
      trg = fst->AddState();
      fst->AddArc (src, fst::StdArc (wid, wid, fst::StdArc::Weight::One(), trg) );
      src = trg;
    }
    statemapper[h] = trg;
  }
};

}
} // end namespaces

#endif //TASK_LMBR_APPLYPOSTERIORS
