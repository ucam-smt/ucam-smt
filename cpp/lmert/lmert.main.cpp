#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main.lmert.hpp>

int main ( int argc,  const char* argv[] ) {
  using namespace ucam::util;
  using namespace ucam::fsttools;
  using namespace ucam::lmert;
  using namespace HifstConstants;
  initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================", "=====================" ) );
  std::string extTok(rg.exists(kLmertExternalTokenizer) ? rg.getString(kLmertExternalTokenizer) : "");
  std::string wMap(rg.exists(kLmertWMap) ? rg.getString(kLmertWMap) : "");
  std::string refFiles;
  bool intRefs;
  if (rg.exists(kLmertWordRefs)) {
    refFiles = rg.getString(kLmertWordRefs);
    intRefs = false;
  }
  if (rg.exists(kLmertIntRefs)) {
    refFiles = rg.getString(kLmertIntRefs);
    intRefs = true;
  } 
  unsigned int bleuCacheSize = rg.get<int>(kLmertBleuCacheSize);
  BleuScorer bleuScorer(refFiles, extTok, bleuCacheSize, intRefs, wMap);
  TuneSet< TupleArc32 > tuneSet(rg);
  PARAMS32 lambda = ParseParamString<float>(rg.getString(kLmertInitialParams));
  if(rg.getString(kLmertDirection).size() > 0){
    PARAMS32 direction = ParseParamString<float>(rg.getString(kLmertDirection));
    SingleLineSearch< TupleArc32 > rls(rg, tuneSet, bleuScorer, lambda, direction);
  }else{
    RandomLineSearch< TupleArc32 > rls(rg, tuneSet, bleuScorer, lambda);
  }
  LINFO(bleuScorer.CacheStats());
  FORCELINFO (argv[0] << " finished!");
}
