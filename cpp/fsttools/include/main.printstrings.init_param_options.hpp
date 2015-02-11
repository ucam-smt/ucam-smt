#ifndef MAIN_NWCP_INITPARAMOPTIONS_HPP
#define MAIN_NWCP_INITPARAMOPTIONS_HPP

/** \file
 *    \brief To initialize boost parameter options
 * \date 21-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace util {

namespace po = boost::program_options;

/**
 *\brief Function to initialize boost program_options module with command-line and config file options.
 * Note that both the config file and the command line options are parsed. This means that whatever the source
 * of the parameter it is equally safe to use, i.e. the expected type (int, string, ...)
 * as defined in the options should be guaranteed a priori.
 * This function is typically used with RegistryPO class, which will contain all relevant variables to share
 * across all task classes.
 * \param argc number of command-line options, as generated for the main function
 * \param argv standard command-line options, as generated for the main function
 * \param vm boost variable containing all parsed options.
 * \return void
 */

inline void init_param_options ( int argc, const char* argv[],
                                 po::variables_map *vm ) {
  try {
    po::options_description desc ( "Command-line/configuration file options" );
    desc.add_options()
    ( HifstConstants::kRangeExtended.c_str(), po::value<std::string>(),
      "Indices of sentences to translate" )
    ( HifstConstants::kLabelMapExtended.c_str(),
      po::value<std::string>()->default_value (""),
      "Map labels to words with this map file" )
    ( HifstConstants::kInput.c_str(), po::value<std::string>()->default_value ("-"),
      "Read original lattice from [file]" )
    ( HifstConstants::kOutput.c_str(),
      po::value<std::string>()->default_value ("-"),
      "Write result" )
    ( HifstConstants::kNbestExtended.c_str(),
      po::value<unsigned>()->default_value (1), "Number of hypotheses" )
    ( HifstConstants::kUniqueExtended.c_str(), "Unique strings" )
    ( HifstConstants::kWeightExtended.c_str(), "Print weight" )
    ( HifstConstants::kSparseFormat.c_str(), "Print weight in sparse format" )
    ( HifstConstants::kSparseDotProduct.c_str(), "Print dot product" )
    ( HifstConstants::kHifstSemiring.c_str(),
      po::value<std::string>()->default_value ("stdarc"),
      "Choose between stdarc, lexstdarc, and tuplearc (for the tropical sparse tuple arc semiring)")
    ( HifstConstants::kTupleArcWeights.c_str(), po::value<std::string>(),
      "Tropic sparse tuple arc weights. "
      "Comma-separated floats. This needs to be set when the option --semiring=tuplearc is chosen.")
        ( HifstConstants::kPrintOutputLabelsExtended.c_str(),
      "Prints output labels instead of input labels" )
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
} // end namespaces

#endif
