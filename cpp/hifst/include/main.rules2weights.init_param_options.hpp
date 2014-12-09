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

#include <main.rules2weights.init_param_options_common.hpp>

/** \file
 * \brief To initialize boost parameter options
 * \date 5-12-2014
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
    initRules2WeightsOptions(desc);
    parseOptionsGeneric (desc, vm, argc, argv);
    checkRules2Weightptions(vm);
    if ( vm->count ( kRulesToWeightsLoadGrammar.c_str() ) ) {
      LDEBUG ( kRulesToWeightsLoadGrammar <<
               ( *vm ) [kRulesToWeightsLoadGrammar.c_str()].as<std::string>() );
    } else {
      LERROR ( "parameter " << kRulesToWeightsLoadGrammar << " not defined" );
      exit ( EXIT_FAILURE );
    }
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
