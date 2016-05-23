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

#ifndef TASK_LMBR_HPP

/**
 * \file
 * \brief Lattice MBR task -- integrates lattice mbr as a task that can be used standalone (implemented) or included e.g. as another task in hifst.
 * \date 10-01-2013
 * \author Gonzalo Iglesias
 */

#include "task.lmbr.common.hpp"
#include "task.lmbr.computeposteriors.hpp"
#include "task.lmbr.applyposteriors.hpp"

namespace ucam {
namespace lmbr {

///Lattice MBR task
template <class Data>
class LmbrTask: public ucam::util::TaskInterface<Data> {

 private:
  typedef fst::NGramList NGramList;

  ucam::util::NumberRange<float> alpha_, wps_;

  bool onebest_;

  unsigned minorder_;
  unsigned maxorder_;

  //Lmbr output...
  fst::VectorFst<fst::StdArc> lmbroutput_;

  NGramToStateMapper statemapper;
  Wlist vocab;
  std::vector<NGramList> ngrams;
  Theta theta_;
  NGramToPosteriorsMapper posteriors;

  //Key to access evidence space
  const std::string evidencespacekey_;
  //Key to access hypotheses space
  const std::string hypothesesspacekey_;
  //key to write lmbr lattice output
  const std::string lmbroutputkey_;

  //For pre-pruning.
  float ppweight_;

  bool loadlexstdarc_;

//public methods
 public:
  ///Constructor using multiple keys that can be arranged so to use different parameter names
  LmbrTask (const ucam::util::RegistryPO& rg,
            const std::string& evidencespacekey = HifstConstants::kLmbrLoadEvidencespace,
            const std::string& hypothesesspacekey =
              HifstConstants::kLmbrLoadHypothesesspace,
            const std::string& lmbroutputkey = HifstConstants::kLmbrWritedecoder,
            const std::string& writeonebestkey = HifstConstants::kLmbrWriteonebest,
            const std::string& alphakey = HifstConstants::kLmbrAlpha,
            const std::string& wpskey = HifstConstants::kLmbrWps,
            const std::string& minorder = HifstConstants::kLmbrMinorder,
            const std::string& maxorder = HifstConstants::kLmbrMaxorder,
            const std::string& unigramprecisionkey = HifstConstants::kLmbrP,
            const std::string& precisionratiokey = HifstConstants::kLmbrR,
            const std::string& numberunigramtokenskey = HifstConstants::kLmbrT,
            const std::string& preprunekey = HifstConstants::kLmbrPreprune,
            const std::string& lexstdarckey = HifstConstants::kLmbrLexstdarc
           ) :
    evidencespacekey_ (evidencespacekey),
    hypothesesspacekey_ (hypothesesspacekey),
    lmbroutputkey_ (lmbroutputkey),
    minorder_ (rg.get<unsigned> (minorder) ),
    maxorder_ (rg.get<unsigned> (maxorder) ),
    alpha_ (rg, alphakey),
    wps_ (rg, wpskey),
    ppweight_ (rg.get<float> (preprunekey) ),
    onebest_ (rg.exists (writeonebestkey) ),
    loadlexstdarc_ (rg.exists (lexstdarckey) ),
    theta_ (rg.get<float> (unigramprecisionkey),
            rg.get<float> (precisionratiokey),
            rg.get<float> (numberunigramtokenskey),
            rg.get<unsigned> (minorder),
            rg.get<unsigned> (maxorder)
           ) {
    if (minorder_ <  1 || maxorder_ <  1) {
      std::cerr << "error: 'minorder' and/or 'maxorder' < 1 \n";
      exit (1);
    }
    if (minorder_ > 10 || maxorder_ > 10) {
      std::cerr << "error: 'minorder' and/or 'maxorder' > 10\n";
      exit (1);
    }
    if (minorder_ > maxorder_) {
      std::cerr << "error: 'minorder' > 'maxorder'\n";
      exit (1);
    }
    LINFO ( "min order=" << minorder_ );
    LINFO ( "max order=" << maxorder_ );
    ngrams.resize (maxorder_ + 1);
  };

  ///Run method inherited from TaskInterface
  bool run (Data& d) {
    FORCELINFO ("applying Lattice MBR , sentence " << d.sidx );
    lmbroutput_.DeleteStates();
    //Identify evidence space and hypothesis space. Could be the same.
    fst::VectorFst<fst::StdArc>* fstevd = NULL;
    fst::VectorFst<fst::StdArc> aux;
    fst::RelabelUtil<fst::StdArc> ru;
    if (d.fsts.find (evidencespacekey_) != d.fsts.end() ) {
      if (loadlexstdarc_) { //We have lexstd, no need for second weight at this point, so we just map down to tropical.
        fst::Map<fst::LexStdArc, fst::StdArc> (*
                                               (static_cast<fst::VectorFst<fst::LexStdArc> *> (d.fsts[evidencespacekey_]) ),
                                               &aux,
                                               fst::LexStdToStdMapper (1) );
        d.fsts[evidencespacekey_] =
          &aux; //rewriting this pointer, would not be accesible for future tasks.
      }
      fstevd = static_cast<fst::VectorFst<fst::StdArc> *> (d.fsts[evidencespacekey_]);
      //Take out OOVs and DRs from evidence space.
      ru.addIPL (DR, EPSILON)
      .addOPL (DR, EPSILON)
      .addIPL (OOV, EPSILON)
      .addOPL (OOV, EPSILON)
      .addIPL (SEP, EPSILON)
      .addOPL (SEP, EPSILON)
      (fstevd);
      fst::Determinize (fst::RmEpsilonFst<fst::StdArc> (*fstevd), fstevd);
      fst::Minimize (fstevd);
      if (ppweight_ != std::numeric_limits<float>::max() ) {
        LINFO ("Pruning evidence space, weight=" << ppweight_);
        fst::Prune (fstevd, ppweight_);
        fst::Determinize (fst::RmEpsilonFst<fst::StdArc> (*fstevd),
                          fstevd); //Awesome, repeat this.
        fst::Minimize (fstevd);
      }
    }
    if (fstevd == NULL) {
      LINFO ("No evidence space provided. Skipping LMBR!");
      return false;
    }
    fst::VectorFst<fst::StdArc>* fsthyp = NULL;
    fst::VectorFst<fst::StdArc> aux2;
    if (d.fsts.find (hypothesesspacekey_) != d.fsts.end() ) {
      if (loadlexstdarc_) { //We have lexstd, no need for second weight at this point, so we just map down to tropical.
        fst::Map<fst::LexStdArc, fst::StdArc> (*
                                               (static_cast<fst::VectorFst<fst::LexStdArc> *> (d.fsts[hypothesesspacekey_]) ),
                                               &aux2, fst::LexStdToStdMapper (1) );
        d.fsts[hypothesesspacekey_] =
          &aux2; //rewriting this pointer, would not be accesible for future tasks...
      } else {
        fsthyp = static_cast<fst::VectorFst<fst::StdArc> * >
                 (d.fsts[hypothesesspacekey_]);
      }
    } else fsthyp = fstevd;
    //Take out OOVs and DRs from hypotheses space.
    ru (fsthyp);
    fst::RmEpsilon (fsthyp);
    //Extract ngrams from evidence space
    unsigned count = extractNGrams (*fstevd, ngrams, minorder_, maxorder_);
    LINFO ( count << " ngrams extracted (evidence space)");
    if (fsthyp != NULL && fsthyp != fstevd) {
      //Additionally, extract ngrams from hypotheses space
      unsigned count_hs = extractNGrams (*fsthyp, ngrams, minorder_, maxorder_);
      LINFO ( count_hs << " ngrams extracted (hypotheses space)");
    }
    //Lattice vocabulary...
    extractSourceVocabulary (*fstevd, &vocab);
    LINFO ("Fast posterior computing");
    ComputePosteriors cp (ngrams);
    for ( alpha_.start(); !alpha_.done (); alpha_.next() ) {
      LINFO ( "scaling weights by " << std::fixed << std::setprecision (
                4) << alpha_() );
      boost::scoped_ptr< fst::VectorFst<fst::StdArc> > scaledfst (FstScaleWeights (
            fstevd, alpha_() ) );
      cp (scaledfst.get() );
      NGramToPosteriorsMapper& pst = cp.getPosteriors();
      ApplyPosteriors ap (ngrams, pst, theta_);
      fst::Map (*fsthyp, &lmbroutput_, fst::RmWeightMapper<fst::StdArc>() );
      boost::scoped_ptr<fst::VectorFst<fst::StdArc> > lmbrlat (ap (lmbroutput_) );
      fst::VectorFst<fst::StdArc> original (*lmbrlat);
      for ( wps_.start(); !wps_.done (); wps_.next() ) {
        fst::Map (original, &lmbroutput_, fst::TimesMapper<fst::StdArc> (wps_() ) );
				std::string hyp;
				FstGetBestStringHypothesis(lmbroutput_, hyp);
        LINFO ("wps=" << wps_() << ":" << hyp);
        if (onebest_) {
          //    *d.lmbronebest+="alpha=" + toString(alpha_())+ " wps=" + toString(wps_()) + " " + toString(d.sidx)+ ":" + hyp + "\n";
          d.lmbronebest->alpha.push_back (alpha_() );
          d.lmbronebest->wps.push_back (wps_() );
          d.lmbronebest->hyp.push_back (hyp);
        }
      }
    }
    //Last alpha,wps gets to the lmbr lattice output. This is fine for single alpha/wp values.
    //If you want to tune and dump a particular lmbr decoder _at the same time_ with a particular alpha and wps,
    //put these values at the end of the range.
    d.lmbronebest->idx = d.sidx;
    d.fsts[lmbroutputkey_] = &lmbroutput_;
    ngrams.clear();
    LINFO ("LMBR finished");
    return false;
  };

 private:

};

}
}  // end namespaces
#endif
