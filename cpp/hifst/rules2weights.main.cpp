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

#define HIFST

/**
 * \file
 * \brief Alignment lattices to sparse vector weight lattices
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

///Include all necessary headers here.
#include <main.rules2weights.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
//#include <main-run.rules2weights.hpp>
#include <common-helpers.hpp>

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

    if (arc.nextstate == -1 ) {
      std::cerr << "FSTATE weight=" << arc.weight << std::endl;
    }

    Weight nw(arc.weight.DefaultValue()); // new weights;
    for (fst::SparseTupleWeightIterator<FeatureWeight32, int> it ( arc.weight )
             ; !it.Done()
             ; it.Next() ) {
      if (it.Value().first > (int) lmOffset_ ) {
        std::cerr << "Skipping: " << it.Value().first << "=>" << it.Value().second << std::endl;
        continue;
      }
      if (it.Value().first < 0 ) {
        WeightsTableIt itx = d_->weights->find(-it.Value().first );
        if (itx == d_->weights->end()) {
          std::cerr << "RULE NOT FOUND:" << -it.Value().first  << std::endl;
          exit(EXIT_FAILURE);
        }
        Weight aux = itx->second;
        for (fst::SparseTupleWeightIterator<FeatureWeight32, int> auxit ( aux )
                 ; !auxit.Done()
                 ; auxit.Next() ) {
          nw.Push(auxit.Value().first, auxit.Value().second.Value() * it.Value().second.Value());
        }
        continue;
      }
      // if weight is positive and index less than numlms, copy as-is.
      nw.Push(it.Value().first, it.Value().second);
    }
    return ToArc ( arc.ilabel, arc.olabel, nw, arc.nextstate );
  }
};





void myMappingProcedure(fst::VectorFst<TupleArc32> *mfst
                        , ucam::hifst::RuleIdsToSparseWeightLatsData<> &d
                        , unsigned lmOffset) {
  RulesToWeightsMapperObject m(d, lmOffset);
  GenericArcAutoMapper<TupleArc32, RulesToWeightsMapperObject> gam(m);
  fst::Map(mfst, gam);
}


// Note: The semiring is always Tuple32.
void ucam::fsttools::MainClass::run() {
  using namespace HifstConstants;
  using namespace ucam::hifst;
  unsigned offset = rg_->get<unsigned>(kRulesToWeightsNumberOfLanguageModels);
  LINFO("LM offset=" << offset);
  LoadSparseWeightsTask<RuleIdsToSparseWeightLatsData<> > p(*rg_, offset);
  RuleIdsToSparseWeightLatsData<> d;
  p.run(d);
  // Read lattices and do the mapping procedure
  using namespace fst;
  using namespace HifstConstants;
  using namespace ucam::util;
  typedef TupleArc32 Arc;
 PatternAddress<unsigned> pi (rg_->get<std::string> (kSparseweightvectorlatticeLoadalilats ) );
  PatternAddress<unsigned> po (rg_->get<std::string> (kSparseweightvectorlatticeStore ) );
  for ( IntRangePtr ir (IntRangeFactory ( *rg_, kRangeOne ) );
        !ir->done();
        ir->next() ) {
    VectorFst<Arc> *mfst = VectorFstRead<Arc> (pi (ir->get() ) );
    LINFO("Reading: " << pi (ir->get() ) );
    myMappingProcedure(mfst, d, offset);
    std::string auxs = po (ir->get() );
    LINFO("Writing: " << auxs);
    FstWrite<Arc> (*mfst, auxs);
  }

}



/**
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks     Main function. Runs alilats2splats tool. See main-run.alilats2splats.hpp
 */
int main ( int argc, const char *argv[] ) {
  ucam::fsttools::MainClass(argc,argv).run();
  return 0;
}
