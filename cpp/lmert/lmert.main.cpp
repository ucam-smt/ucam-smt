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

  PARAMS32 lambda = GetLambda(rg);
  BleuScorer bleuScorer(rg);
  TuneSet< TupleArc32 > tuneSet(rg);
  RandomLineSearch< TupleArc32 > rls(rg, tuneSet, bleuScorer, lambda); 
  FORCELINFO ( argv[0] << " finished!" );
}
