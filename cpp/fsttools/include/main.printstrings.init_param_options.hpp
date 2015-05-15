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
  using namespace HifstConstants;

  try {
    po::options_description desc ( "Command-line/configuration file options" );
    desc.add_options()
    ( kRangeExtended.c_str(), po::value<std::string>(),
      "Indices of sentences to translate" )
    ( kLabelMapExtended.c_str(),
      po::value<std::string>()->default_value (""),
      "Map labels to words with this map file" )
    ( kInput.c_str(), po::value<std::string>()->default_value ("-"),
      "Read original lattice from [file]" )
    ( kOutput.c_str(),
      po::value<std::string>()->default_value ("-"),
      "Write result" )
    ( kNbestExtended.c_str(),
      po::value<unsigned>()->default_value (1), "Number of hypotheses" )
    ( kUniqueExtended.c_str(), "Unique strings" )
    ( kWeightExtended.c_str(), "Print weight" )
    ( kWeightPrecision.c_str(), po::value<unsigned>()->default_value(6), "Weight precision ")
    ( kSparseFormat.c_str(), "Print weight in sparse format" )
    ( kSentBleu.c_str(), "compute sentence level bleu (blue+1)" )
    ( kSuppress.c_str(), "don't print hyps" )
    ( kLibLinRankFormat.c_str(), "liblinear ranking format (apro)" )
    ( kSparseDotProduct.c_str(), "Print dot product" )
    ( kIntRefs.c_str(), po::value<std::string>(), "reference strings (integers)" )
    ( kWordRefs.c_str(), po::value<std::string>(), "reference strings (words)" )
    ( kExternalTokenizer.c_str(), po::value<std::string>(), "external tokenization script" )
    ( kSparseFormat.c_str(), "Print weight in sparse format" )
    ( kSparseDotProduct.c_str(), "Print dot product" )
    ( kHifstSemiring.c_str(),
      po::value<std::string>()->default_value ("stdarc"),
      "Choose between stdarc, lexstdarc, and tuplearc (for the tropical sparse tuple arc semiring)")
    ( kTupleArcWeights.c_str(), po::value<std::string>(),
      "Tropic sparse tuple arc weights. "
      "Comma-separated floats. This needs to be set when the option --semiring=tuplearc is chosen.")
    ( kPrintOutputLabelsExtended.c_str(),
      "Prints output labels instead of input labels" )
    ( kPrintInputOutputLabels.c_str(),
      "Prints both input and  output labels" )
    ( kIntersectionWithHypothesesLoad.c_str(), po::value<std::string>()->default_value("")
      , "Loads file with hypotheses (input or output depending on parameter to print output labels "
      " and generates an FSA that intersects "
      " with input machine before dumping hypothesis. File can be either an FSA "
      " or a plaintext file with one line per hypothesis (words must be integer mapped)"
      )
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
