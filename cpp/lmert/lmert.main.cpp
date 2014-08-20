#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main.lmert.hpp>
#include <bleu.hpp>
#include <tuneset.hpp>
#include <lmert.hpp>
#include <lineoptimize.hpp>
#include <randomlinesearch.hpp>

int main (int argc,  const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================", "=====================" ) );

  if (rg.get<std::string> (HifstConstants::kHifstSemiring.c_str() ) != HifstConstants::kHifstSemiringTupleArc) {
    LERROR("Only HifstSemiringTupleArc allowed, at present");
  }
  const std::string& tuplearcWeights = rg.exists(HifstConstants::kTupleArcWeights) ? rg.get<std::string> (HifstConstants::kTupleArcWeights.c_str() ) : "";
  if (tuplearcWeights.empty()) {
    LERROR("weights not set - proceeding");
  }
  PARAMS32 lambda = ucam::util::ParseParamString<float> (tuplearcWeights);

  BleuScorer bleuScorer(rg);
  TuneSet< TupleArc32 > tuneSet(rg);
  RandomLineSearch< TupleArc32 > rls(rg, tuneSet, bleuScorer, lambda); 
  FORCELINFO ( argv[0] << " finished!" );
}
