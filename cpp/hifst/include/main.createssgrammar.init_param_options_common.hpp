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
 *    \brief To initialize boost parameter options for createssgrammar tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace util {

namespace po = boost::program_options;

inline void initAllCreateSSGrammarOptions (po::options_description& desc) {
  desc.add_options()
  ( HifstConstants::kRangeExtended.c_str(), po::value<std::string>(),
    "Indices of sentences to process" )
  ( HifstConstants::kNThreads.c_str(), po::value<unsigned>(),
    "Number of threads (trimmed to number of cpus in the machine) " )
  ( HifstConstants::kGrammarLoad.c_str(), po::value<std::string>(),
    "Load a synchronous context-free grammar file" )
  ( HifstConstants::kGrammarFeatureweights.c_str(),
    po::value<std::string>()->default_value ( "1" ),
    "One or more scales. Must match the number of features in the grammar" )
  ( HifstConstants::kGrammarStorepatterns.c_str(),
    po::value<std::string>()->default_value ( "" ),
    "Store a file containing patterns" )
  ( HifstConstants::kGrammarStorentorder.c_str(),
    po::value<std::string>()->default_value ( "" ),
    "Store a file containing non-terminal table" )
  ( HifstConstants::kSourceLoad.c_str(),
    po::value<std::string>()->default_value ( "-" ),
    "Source text file -- this option is ignored in server mode" )
  ( HifstConstants::kPreproTokenizeEnable.c_str(),
    po::value<std::string>()->default_value ( "no" ),
    "Tokenize input (yes|no) -- NOT IMPLEMENTED" )
  ( HifstConstants::kPreproTokenizeLanguage.c_str(),
    po::value<std::string>()->default_value ( "" ), "NOT IMPLEMENTED" )
  ( HifstConstants::kPreproAddsentencemarkers.c_str(),
    "Add sentence markers to tokenized input" )
  ( HifstConstants::kPreproWordmapLoad.c_str(),
    po::value<std::string>()->default_value ( "" ),
    "Loads a map file that will be used to integer-map the words of the source sentence. " )
  ( HifstConstants::kPatternstoinstancesMaxspan.c_str(),
    po::value<unsigned>()->default_value ( 10 ), "Maximum span" )
  ( HifstConstants::kPatternstoinstancesGapmaxspan.c_str(),
    po::value<unsigned>()->default_value ( 9 ), "Maximum gap span" )
  ( HifstConstants::kPatternstoinstancesStore.c_str(),
    po::value<std::string>()->default_value ( "" ), "wordmap file" )
  ( HifstConstants::kSsgrammarStore.c_str(),
    po::value<std::string>()->default_value ( "" ),
    "Store sentence-specific grammar" )
  ( HifstConstants::kSsgrammarAddoovsEnable.c_str(),
    po::value<std::string>()->default_value ( "no" ), "Add oov rules (yes|no)" )
  ( HifstConstants::kSsgrammarAddoovsSourcedeletions.c_str(),
    po::value<std::string>()->default_value ( "yes" ),
    "If OOVs added, then delete oovs rather than pass them through (yes|no)" )
  ;
}

inline void checkCreateSSGrammarOptions (po::variables_map *vm) {
  if ( vm->count ( HifstConstants::kGrammarLoad.c_str() ) ) {
    LDEBUG ( HifstConstants::kGrammarLoad << "=" <<
             ( *vm ) [HifstConstants::kGrammarLoad.c_str()].as<std::string>() );
  } else {
    LERROR ( HifstConstants::kGrammarLoad << " not defined" );
    exit ( EXIT_FAILURE );
  }
  if ( vm->count ( HifstConstants::kSourceLoad.c_str() ) ) {
    LDEBUG ( HifstConstants::kSourceLoad << "=" <<
             ( *vm ) [HifstConstants::kSourceLoad.c_str()].as<std::string>() );
  } else if ( vm->count ( HifstConstants::kServerEnable.c_str() ) ) {
    LERROR ( HifstConstants::kSourceLoad <<
             " not defined, mandatory unless running on server mode " );
    exit ( EXIT_FAILURE );
  }
}

}
} // end namespaces
