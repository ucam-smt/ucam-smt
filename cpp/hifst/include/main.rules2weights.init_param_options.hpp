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
    desc.add_options()
    ( kRangeExtended.c_str(),
      po::value<std::string>()->default_value ("1"),
      "Indices of sentences to translate" )
    ( kNThreads.c_str(), po::value<unsigned>(),
      "Number of threads (trimmed to number of cpus in the machine) " )
     ( kFeatureweights.c_str()
       , po::value<std::string>()->default_value ( "" )
       , "Feature weights applied in hifst. This is a comma-separated sequence "
       "of language model(s) and grammar feature weights.\n"
       "IMPORTANT: If this option is not empty string, then it will override "
       "any values in lm.featureweights and ruleflowerlattice.featureweights"
     )
    ( kRuleflowerlatticeFilterbyalilats.c_str(),
      "Filter the flower lattice with the vocabulary of the alignment lattices" )
    ( kRuleflowerlatticeLoad.c_str(), po::value<std::string>(),
      "Load a synchronous context-free grammar file" )
    // ( kRuleflowerlatticeStore.c_str(),
    //   po::value<std::string>()->default_value ( "" ), "Store the fst (SparseWeight)" )
    // ( kRuleflowerlatticeFeatureweights.c_str(),
    //   po::value<std::string>()->default_value ( "1" ),
    //   "One or more feature weights. Must match the number of features in the grammar" )
    ( kSparseweightvectorlatticeLoadalilats.c_str() ,
      po::value<std::string>(), "Load hifst sparse weight translation lattice" )
    ( kSparseweightvectorlatticeStore.c_str() ,
      po::value<std::string>()->default_value ( "" ),
      "Store the fst (SparseWeight) containing a vector of weights " )
    // ( kSparseweightvectorlatticeStripSpecialEpsilonLabels.c_str() ,
    //   po::value<std::string>()->default_value ( "no" ),
    //   "Strip any special Hifst epsilon labels (e.g. oov, deletion rule, ...)" )
    // ( kSparseweightvectorlatticeStorenbestfile.c_str(),
    //   po::value<std::string>()->default_value ( "" ),
    //   "Store the fst (SparseWeight) containing a vector of weights " )
    // ( kSparseweightvectorlatticeWordmap.c_str(),
    //   po::value<std::string>()->default_value ( "" ),
    //   "Use wordmap when dumping nbest list (to use with storenbestfile option )" )
    // ( kSparseweightvectorlatticeStorefeaturefile.c_str(),
    //   po::value<std::string>()->default_value ( "" ),
    //   "Store the fst (SparseWeight) containing a vector of weights " )
    // ( kSparseweightvectorlatticeFirstsparsefeatureatindex.c_str(),
    //   po::value<uint>()->default_value ( 50 ),
    //   "Number for which the feature output will printed in sparse format (weight_1@position_1 ... weight_n@position_n" )
        ( kRulesToWeightsNumberOfLanguageModels.c_str()
        , po::value<unsigned>()->default_value ( 1 )
        , "Number of language models" )
    ;
    //    initCommonApplylmOptions (desc); // Add generic language model options
    parseOptionsGeneric (desc, vm, argc, argv);
    if ( vm->count ( kRuleflowerlatticeLoad.c_str() ) ) {
      LDEBUG ( "ruleflowerlattice.load=" <<
               ( *vm ) [kRuleflowerlatticeLoad.c_str()].as<std::string>() );
    } else {
      LERROR ( "parameter ruleflowerlattice.load not defined" );
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
