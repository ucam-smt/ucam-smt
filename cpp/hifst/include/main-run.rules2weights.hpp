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

#pragma once

/**
 * \file
 * \brief Implements lats2splats tool
 * \date 10-12-2014
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {


struct RulesToWeightsMapperObject {
  typedef TupleArc32 FromArc;
  typedef TupleArc32 ToArc;
  typedef ToArc::Weight Weight;
  ucam::hifst::RuleIdsToSparseWeightLatsData<> *d_;
  unsigned lmOffset_;
  typedef ucam::hifst::RuleIdsToSparseWeightLatsData<>::WeightsTableIt WeightsTableIt;
  explicit RulesToWeightsMapperObject(ucam::hifst::RuleIdsToSparseWeightLatsData<> &d
                                      , unsigned lmOffset )
      : d_(&d)
      , lmOffset_(lmOffset)
  { }

  ToArc operator()(FromArc const &arc) const {
    if ( arc.weight == FromArc::Weight::Zero() ) //irrelevant labels
      return ToArc ( arc.ilabel, arc.olabel, ToArc::Weight::Zero(), arc.nextstate );

    // minimize weight list (this semiring allows for repeated indices!
    std::map<int, float> wm;
    for (fst::SparseTupleWeightIterator<FeatureWeight32, int> it ( arc.weight )
             ; !it.Done()
             ; it.Next() ) {
      if (it.Value().first > (int) lmOffset_ ) {
        continue;
      }
      if (it.Value().first < 0 ) {
        WeightsTableIt itx = d_->weights->find(-it.Value().first );
        if (itx == d_->weights->end()) {
          std::cerr << "RULE NOT FOUND:" << -it.Value().first  << "," << d_->weights->size() << std::endl;
          exit(EXIT_FAILURE);
        }
        Weight aux = itx->second;
        for (fst::SparseTupleWeightIterator<FeatureWeight32, int> auxit ( aux )
                 ; !auxit.Done()
                 ; auxit.Next() ) {
	  wm[auxit.Value().first] += auxit.Value().second.Value() * it.Value().second.Value();
        }
        continue;
      }
      wm[it.Value().first] += it.Value().second.Value();
    }
    // finally create the weights ...
    Weight nw(arc.weight.DefaultValue()); // new weights;
    for (std::map<int, float>::const_iterator itx = wm.begin()
	   ; itx != wm.end()
	   ; ++itx ) {
      nw.Push(itx->first, itx->second);	  
    }
    return ToArc ( arc.ilabel, arc.olabel, nw, arc.nextstate );
  }
};

/**
 * \brief Full single-threaded Alignment lattices to Sparse lattices
 */

class SingleThreadededRulesToWeightsSparseLatsTask: public  ucam::util::TaskInterface<RuleIdsToSparseWeightLatsData<> > {
 private:
  typedef RuleIdsToSparseWeightLatsData<> Data;
  const ucam::util::RegistryPO& rg_;
 public:
  SingleThreadededRulesToWeightsSparseLatsTask ( const ucam::util::RegistryPO& rg )
    : rg_ ( rg ) 		       
  {};

  bool run ( Data& d ) {
    using namespace HifstConstants;
    using namespace ucam::hifst;
    using namespace fst;
    using namespace ucam::util;

    unsigned offset = 1;
    if (rg_.exists(kRulesToWeightsNumberOfLanguageModels)) {
      offset = rg_.get<unsigned>(kRulesToWeightsNumberOfLanguageModels);
    } else if (rg_.exists(kLmFeatureweights)) {
      offset = rg_.getVectorString(kLmFeatureweights).size();
    } else {
      LERROR("Cannot determine parameter to find the number of language models! (" << kRulesToWeightsNumberOfLanguageModels << "," << kLmFeatureweights << ")");
      exit(EXIT_FAILURE);
    }
    LINFO("#LMs =" << offset);

    std::string alilats;
    std::string range = kRangeOne;
    if (rg_.exists(kRulesToWeightsLoadalilats)) {
      alilats = kRulesToWeightsLoadalilats;
    } else if (rg_.exists(kHifstLatticeStore)) {
      alilats = kHifstLatticeStore;
      range = kRangeInfinite; // hifst doesn't have range.
    } else {
      LERROR("Could not determine parameter to find input lattices ! (" << kRulesToWeightsLatticeFilterbyAlilats << "," << kHifstLatticeStore << ")" );
      exit(EXIT_FAILURE);
    }
    std::string loadgrammar;
    if (rg_.exists(kRulesToWeightsLoadGrammar)) {
      loadgrammar=kRulesToWeightsLoadGrammar;
    } else if (rg_.exists(kGrammarLoad)) {
      loadgrammar=kGrammarLoad;
    } else {
      LERROR("Grammar parameter is unavailable ! (" << kRulesToWeightsLoadGrammar << "," << kGrammarLoad << ")" );
      exit(EXIT_FAILURE);
    }

    LoadSparseWeightsTask<Data> p(rg_, offset, alilats, loadgrammar);
    p.run(d);

    typedef TupleArc32 Arc;
    PatternAddress<unsigned> pi (rg_.get<std::string> (alilats ) );
    PatternAddress<unsigned> po (rg_.get<std::string> (kRulesToWeightsLatticeStore ) );
    for ( IntRangePtr ir (IntRangeFactory ( rg_, range ) );
          !ir->done();
          ir->next() ) {

      if (!ucam::util::fileExists(pi (ir->get() ))
          && range == kRangeInfinite) {
        // silently finish
        break;
      }
      VectorFst<Arc> *mfst = VectorFstRead<Arc> (pi (ir->get() ) );
      FORCELINFO("Reading: " << pi (ir->get() ) );
      myMappingProcedure(mfst, d, offset);
      std::string auxs = po (ir->get() );
      FORCELINFO("Writing: " << auxs);
      FstWrite<Arc> (*mfst, auxs);
    }
  };

  inline bool operator() () {
    Data d;
    return run ( d );
  };

 private:

  void myMappingProcedure(fst::VectorFst<TupleArc32> *mfst
                          , Data &d
                          , unsigned lmOffset) {

    RulesToWeightsMapperObject m(d, lmOffset);
    fst::GenericArcAutoMapper<TupleArc32, RulesToWeightsMapperObject> gam(m);
    fst::Map(mfst, gam);
  }

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadededRulesToWeightsSparseLatsTask );
};

}} // end namespaces


