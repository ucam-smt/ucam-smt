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

#include <main.applylm.init_param_options_common.hpp>

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

inline void init_param_options ( int argc, const char* argv[]
                                 , po::variables_map *vm ) {
  using namespace HifstConstants;
  using namespace std;
  try {
    po::options_description desc ( "Command-line/configuration file options" );
    initCommonApplylmOptions (desc);
    desc.add_options()
    ( kRange.c_str(), po::value<string>(),
      "Indices of lattices to rescore" )
    ( kNThreads.c_str(), po::value<uint>(),
      "Number of threads (trimmed to number of cpus in the machine) " )
    ( kLatticeLoad.c_str(), po::value<string>(),
      "Read original lattice from [file]" )
    ( kLatticeLoadDeleteLmCost.c_str(),
      "If working on LexStd, then delete lm cost in the lattices first,"
      " by projecting Weight2 into Weight1." )
    ( kLatticeStore.c_str(),
      po::value<string>()->default_value ( "" ),
      "Write  lattice with lm scores to [file]" )
    (kUseBilingualModel.c_str()
     , po::value<string>()->default_value("no")
     , "Use bilingual models. Only nplm model supported"
     )
    (kUseBilingualModelSourceSize.c_str()
     , po::value<unsigned>()->default_value(3)
     , "Source size. Only 0 or positive odd values accepted. "
     "NPLM model must be trained accordingly! If 0, it will assume NNLM. "
     "Note that 0 is intended to use only for debugging purposes -- this should be equivalent to "
     "(but slower than) --usebilm=no."
     )
    (kUseBilingualModelSourceSentenceFile.c_str()
     , po::value<string>()->default_value("")
     , "Single file containing source sentences. Mandatory if "
     "bilingual models are used!"
     )
    (kTune.c_str()
     , po::value<string>()->default_value("no")
     , "Dumps 1-best hypotheses for different word penalty corrections"
     )
    (kTuneWrite.c_str()
     , po::value<string>()->default_value("")
     , "Where to write 1-best output per word penalty correction (use %%wp%%)"
     )
    (kTuneWordPenaltyRange.c_str()
     , po::value<string>()->default_value("")
     , "Range of word penalties in usual range format (float values)."
     )
    ( kHifstSemiring.c_str()
      , po::value<string>()->default_value
      ( kHifstSemiringStdArc.c_str() )
      , "Choose between lexstdarc or stdarc" )
    ( kStatsWrite.c_str()
      , po::value<string>()->default_value ( "" )
      , "Write speed stats to  [file]" )
    ;
    parseOptionsGeneric (desc, vm, argc, argv);
    checkApplyLmOptions (vm);
  } catch ( exception& e ) {
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
