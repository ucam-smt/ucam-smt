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

#include <main.tunewp.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>

template <class Arc>
class TuneWpMain {
 private:

  const ucam::util::RegistryPO& rg_;
  fst::MakeWeight<Arc> mw_;
 public:
  TuneWpMain (ucam::util::RegistryPO const& rg) :
    rg_ (rg) {
  };
  void operator () () {
    using fst::WordPenaltyMapper;
    using fst::VectorFstRead;
    using fst::FstWrite;
    using ucam::util::PatternAddress;
    PatternAddress<unsigned> pi (rg_.get<std::string> (HifstConstants::kInput ) );
    PatternAddress<unsigned> po (rg_.get<std::string> (HifstConstants::kOutput ) );
    ucam::util::NumberRange<float> wp ( rg_.get<std::string>
                                        (HifstConstants::kWordPenalty ) );
    //Insert epsilons
    unordered_set<typename Arc::Label> epsilons =
      rg_.getSetNumber<typename Arc::Label> (HifstConstants::kEpsilonLabels);
    unsigned shp = rg_.get<unsigned> (HifstConstants::kNbest);
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_,
                                      HifstConstants::kRangeOne ) );
          !ir->done();
          ir->next() ) {
      fst::VectorFst<Arc> *mfst = VectorFstRead<Arc> (pi (ir->get() ) );
      for ( wp.start();
            !wp.done();
            wp.next() ) {
        fst::Map<Arc, WordPenaltyMapper<Arc> > (mfst,
                                                WordPenaltyMapper<Arc> (mw_ (wp.get() ), epsilons) );
        if (shp < std::numeric_limits<unsigned>::max() ) {
          fst::VectorFst<Arc> aux;
          fst::ShortestPath<Arc> (*mfst, &aux, shp);
          *mfst = aux;
        }
        std::string auxs = po (ir->get() );
        ucam::util::find_and_replace (auxs, "%%wp%%",
                                      ucam::util::toString<float> (wp() ) );
        FstWrite<Arc> (*mfst, auxs);
      }
    }
  };

};

int main (int argc,  const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "=====================" ) )  ;
  if (rg.get<std::string> (HifstConstants::kHifstSemiring.c_str() ) ==
      HifstConstants::kHifstSemiringStdArc) {
    TuneWpMain<fst::StdArc> twp (rg);
    twp();
  } else if (rg.get<std::string> (HifstConstants::kHifstSemiring.c_str() ) ==
             HifstConstants::kHifstSemiringLexStdArc) {
    TuneWpMain<fst::LexStdArc> twp (rg);
    twp();
  } else {
    LERROR ("Unknown type");
  }
  FORCELINFO ( argv[0] << " ends!" );
}

