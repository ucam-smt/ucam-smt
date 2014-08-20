#ifndef MAIN_LMERT_INITPARAMOPTIONS_HPP
#define MAIN_LMERT_INITPARAMOPTIONS_HPP

namespace ucam {
namespace util {
namespace po = boost::program_options;

inline void init_param_options ( int argc, const char* argv[], po::variables_map *vm ) {
  try {
    po::options_description desc ( "Command-line/configuration file options" );
    desc.add_options()
      ( HifstConstants::kRangeExtended.c_str(), po::value<std::string>(),
	"Indices of tuning set sentences" )
      ( HifstConstants::kInputExtended.c_str(), po::value<std::string>(),
	"Fst(s) to count strings (use ? for multiple instances) " )
      ( HifstConstants::kHifstSemiring.c_str(),
	po::value<std::string>()->default_value ("stdarc"),
	"Choose between stdarc, lexstdarc, and tuplearc (for the tropical sparse tuple arc semiring)")
      ( "initial_params", po::value<std::string>(), "Initial parameter value (lambda)")
      ( "refs", po::value<std::string>(), "MERT reference translations" )
      ( "min_gamma", po::value<float>()->default_value(0.0005f), "minimum gamma value for a line segment")
      ( "min_bleu_gain", po::value<float>()->default_value(0.000001f), "minimum accepted bleu gain (stopping criterion)")
      ( "random_seed", po::value<int>()->default_value(time(0)), "seed for randomline search")
      ( "num_threads", po::value<int>()->default_value(1), "number of threads")
      ( "num_random_directions", po::value<int>()->default_value(0), "number of random directions; default is 2xfeature_dim")
      ( "write_params", po::value<std::string>(), "output parameter file")
      ;
    parseOptionsGeneric (desc, vm, argc, argv);
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
