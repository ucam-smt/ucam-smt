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
    ( HifstConstants::kRecaserLmLoad.c_str(), po::value<std::string>(),
      "Scaling factors applied to the language models" )
    ( HifstConstants::kRecaserLmFeatureweight.c_str(),
      po::value<std::string>()->default_value ( "1.0" ),
      "Scaling factors applied to the language models" )
    ( HifstConstants::kRecaserUnimapLoad.c_str(), po::value<std::string>(),
      "Scaling factors applied to the language models (separated by commas) " )
    ( HifstConstants::kRecaserUnimapWeight.c_str(),
      po::value<float>()->default_value ( 1.0f ),
      "Scaling factors applied to the language models " )
    ( HifstConstants::kRecaserPrune.c_str(),
      po::value<std::string>()->default_value ( "byshortestpath,1" ),
      "Choose between byshortestpath,numpaths or byweight,weight" )
    ( HifstConstants::kRecaserInputExtended.c_str(), po::value<std::string>(),
      "Input lattice" )
    ( HifstConstants::kRecaserOutputExtended.c_str(), po::value<std::string>(),
      "Output lattice" )
    ( HifstConstants::kHifstSemiringExtended.c_str(),
      po::value<std::string>()->default_value (
        HifstConstants::kHifstSemiringStdArc.c_str() ),
      "Choose between lexstdarc or stdarc" )
    ;
    parseOptionsGeneric (desc, vm, argc, argv);
    if ( !vm->count ( HifstConstants::kRecaserLmLoad.c_str() ) ) {
      LERROR ( "Language model file not defined" );
      exit ( EXIT_FAILURE );
    }
    if ( !vm->count ( HifstConstants::kRecaserUnimapLoad.c_str() ) ) {
      LERROR ( "Unimap model not defined" );
      exit ( EXIT_FAILURE );
    }
  } catch ( std::exception& e ) {
    std::cerr << "error: " << e.what() << "\n";
    exit ( EXIT_FAILURE );
  } catch ( ... ) {
    std::cerr << "Exception of unknown type!\n";
    exit ( EXIT_FAILURE );
  }
  LINFO ( "Configuration loaded" );
};

}
} // end namespaces
