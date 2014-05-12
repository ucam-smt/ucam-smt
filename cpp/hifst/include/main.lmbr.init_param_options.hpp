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

/** \file
 * \brief To initialize boost parameter options
 * \date 15-10-2012
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
    ( HifstConstants::kRangeExtended.c_str(),
      po::value<std::string>()->default_value ( "1" ),
      "Indices of sentences to translate" )
    ( HifstConstants::kNThreads.c_str(), po::value<unsigned>(),
      "Number of threads (trimmed to number of cpus in the machine) " )
    ( HifstConstants::kLmbrLexstdarc.c_str(),
      "If set, the tool expects input FSAs to be templated over lexicographic<tropical,tropical> semiring (and so maps them to tropical)" )
    ( HifstConstants::kLmbrLoadEvidencespace.c_str(), po::value<std::string>(),
      "Load an FSA containing the evidence space" )
    ( HifstConstants::kLmbrLoadHypothesesspace.c_str(),
      po::value<std::string>()->default_value ( "" ),
      "Load an FSA containing the hypotheses space" )
    ( HifstConstants::kLmbrWritedecoder.c_str(),
      po::value<std::string>()->default_value ( "" ),
      "Write the lmbr FSA output with posteriors applied" )
    ( HifstConstants::kLmbrWriteonebest.c_str(),
      po::value<std::string>()->default_value ( "" ),
      "Write file text with one-best for tunings. Use %%alpha%% and %%wip%% outputs for different alphas and word penalties" )
    ( HifstConstants::kLmbrMinorder.c_str(),
      po::value<unsigned>()->default_value ( 1 ), "Minimum posterior order to apply" )
    ( HifstConstants::kLmbrMaxorder.c_str(),
      po::value<unsigned>()->default_value ( 4 ), "Maximum posterior order to apply" )
    ( HifstConstants::kLmbrAlpha.c_str(),
      po::value<std::string>()->default_value ( "1" ),
      "Scaling factor of normalized evidence space (range of float values)" )
    ( HifstConstants::kLmbrWps.c_str(),
      po::value<std::string>()->default_value ( "0.0" ),
      "Word penalty (range of float values)" )
    ( HifstConstants::kLmbrP.c_str(), po::value<float>(), "Unigram precision" )
    ( HifstConstants::kLmbrR.c_str(), po::value<float>(), "Precision ratio" )
    ( HifstConstants::kLmbrT.c_str(), po::value<float>()->default_value ( 10.0f ),
      "" )
    ( HifstConstants::kLmbrPreprune.c_str(),
      po::value<float>()->default_value ( std::numeric_limits<float>::max() ),
      "Preprune evidence space" )
    ;
    ucam::util::parseOptionsGeneric (desc, vm, argc, argv);
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
