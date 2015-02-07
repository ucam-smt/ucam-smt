#ifndef MAIN_SAMPLEHYPS_INITPARAMOPTIONS_HPP
#define MAIN_SAMPLEHYPS_INITPARAMOPTIONS_HPP

namespace ucam {
namespace util {

namespace po = boost::program_options;

inline void init_param_options ( int argc, const char* argv[],
                                 po::variables_map *vm ) {
  try {
    po::options_description desc ( "Command-line/configuration file options" );
    desc.add_options()
    ( HifstConstants::kWordRefs.c_str(), po::value<std::string>()->default_value(""), "reference translations words" )
    ( HifstConstants::kIntRefs.c_str(), po::value<std::string>()->default_value(""), "reference translations ints" )
    ( HifstConstants::kExternalTokenizer.c_str(), po::value<std::string>()->default_value(""), "external tokenization command" )
    ( HifstConstants::kWordMap.c_str(), po::value<std::string>()->default_value(""), "word map for reference translations" )
    ( HifstConstants::kRangeExtended.c_str(), po::value<std::string>(),
      "Indices of sentences to translate" )
    ( HifstConstants::kInput.c_str(), po::value<std::string>()->default_value ("-"),
      "Read original lattice from [file]" )
    ( HifstConstants::kOutput.c_str(),
      po::value<std::string>()->default_value ("-"),
      "Write result" )
    ( HifstConstants::kNbestExtended.c_str(),
      po::value<unsigned>()->default_value (1), "Number of hypotheses" )
    ( HifstConstants::kSparseFormat.c_str(), "Print weight in sparse format" )
    ( HifstConstants::kAlpha.c_str(), po::value<float>()->default_value(0.05f),
      "Sampling threshold alpha (see PRO paper)")
    ( HifstConstants::kRandomSeed.c_str(), po::value<unsigned>(), "Number of samples per source sentence to return")
    ( HifstConstants::kNSamples.c_str(), po::value<unsigned>()->default_value(50),
      "Number of samples per source sentence to return")
    ( HifstConstants::kNegativeExamples.c_str(), "Include negative examples")
    ( HifstConstants::kDontNegate.c_str(), "Don't negate feature weights")
    ( HifstConstants::kBinaryTarget.c_str(), "Output binary valued target (for PRO)")
    ( HifstConstants::kHifstSemiring.c_str(),
      po::value<std::string>()->default_value ("stdarc"),
      "Choose between stdarc, lexstdarc, and tuplearc (for the tropical sparse tuple arc semiring)")
    ( HifstConstants::kTupleArcWeights.c_str(), po::value<std::string>(),
      "Tropic sparse tuple arc weights. "
      "Comma-separated floats. This needs to be set when the option --semiring=tuplearc is chosen.")
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
