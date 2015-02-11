#ifndef MAIN_LMERT_INITPARAMOPTIONS_HPP
#define MAIN_LMERT_INITPARAMOPTIONS_HPP

#include <constants-lmert.hpp>
namespace ucam {
namespace util {
namespace po = boost::program_options;

// \todo convert all parameter names into const strings
// and reuse these const parameters instead of literals
// in other places of code.
inline void init_param_options ( int argc, const char* argv[],
                                 po::variables_map *vm ) {
  using namespace HifstConstants;
  try {
    po::options_description desc ( "Command-line/configuration file options" );
    desc.add_options()
      ( kRangeExtended.c_str(), po::value<std::string>(),
	"Indices of tuning set sentences" )
      ( kInputExtended.c_str(), po::value<std::string>(),
	"Fst(s) to count strings (use ? for multiple instances) " )
      ( kLmertInitialParams.c_str(), po::value<std::string>(),
	"Initial parameter value (lambda)" )
      ( kLmertWordRefs.c_str(), po::value<std::string>(), "MERT reference translations words" )
      ( kLmertIntRefs.c_str(), po::value<std::string>(), "MERT reference translations ints" )
      ( kLmertMinGamma.c_str(), po::value<float>()->default_value ( 0.000005f ),
	"minimum gamma value for a line segment" )
      ( kLmertMinBleuGain.c_str(), po::value<float>()->default_value ( 0.000001f ),
	"minimum accepted bleu gain (stopping criterion)" )
      ( kLmertRandomSeed.c_str(), po::value<int>()->default_value ( time ( 0 ) ),
	"seed for randomline search" )
      ( kNThreads.c_str() , po::value<int>()->default_value ( 1 ), "number of threads" )
      ( kLmertNumRandomDirections.c_str(), po::value<int>()->default_value ( 10 ),
	"number of random directions; default is 2xfeature_dim" )
      ( kLmertWriteParams.c_str(), po::value<std::string>(), "output parameter file" )
      ( kLmertExternalTokenizer.c_str(), po::value<std::string>(), "external tokenization command" )
      ( kLmertWMap.c_str(), po::value<std::string>(), "Maps idx to word; for use with word references" )
      ( kLmertBleuCacheSize.c_str(), po::value<int>()->default_value(10000), "number of hyps per sentence for which to LRU cache Bleu stats");

    // \todo is this option supported?
    // ( kHifstSemiring.c_str(),
    //   po::value<std::string>()->default_value ( "stdarc" ),
    //   "Choose between stdarc, lexstdarc, "
    //   "and tuplearc (for the tropical sparse tuple arc semiring)" )

    parseOptionsGeneric ( desc, vm, argc, argv );
  } catch ( std::exception& e ) {
    cerr << "error: " << e.what() << "\n";
    exit ( EXIT_FAILURE );
  } catch ( ... ) {
    cerr << "Exception of unknown type!\n";
    exit ( EXIT_FAILURE );
  }

  LINFO ( "Configuration loaded" );
};

}
}  // end namespaces

#endif
