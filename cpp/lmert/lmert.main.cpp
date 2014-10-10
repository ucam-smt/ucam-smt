#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main.lmert.hpp>

int main ( int argc,  const char* argv[] ) {
	using namespace ucam::util;
	using namespace ucam::fsttools;
	using namespace ucam::lmert;
  initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================", "=====================" ) );
  std::string extTok(rg.exists(HifstConstants::kLmertExternalTokenizer) ? rg.getString(HifstConstants::kLmertExternalTokenizer) : "");
  BleuScorer bleuScorer (rg.getString(HifstConstants::kLmertRefs), extTok, rg.get<int>(HifstConstants::kLmertBleuCacheSize));
  TuneSet< TupleArc32 > tuneSet(rg);
  ucam::lmert::PARAMS32 lambda = ucam::util::ParseParamString<float>
    (rg.getString(HifstConstants::kLmertInitialParams));
  RandomLineSearch< TupleArc32 > rls(rg, tuneSet, bleuScorer, lambda);
  LINFO(bleuScorer.CacheStats());
  FORCELINFO (argv[0] << " finished!");
}
