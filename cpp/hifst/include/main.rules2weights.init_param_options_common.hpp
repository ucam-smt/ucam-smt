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


inline void initRules2WeightsOptions (po::options_description& desc
                                      , bool addAllOptions=true) {
  using namespace HifstConstants;
  if (addAllOptions) {
    desc.add_options()
        ( kRangeExtended.c_str(),
          po::value<std::string>()->default_value ("1"),
          "Indices of sentences to process" )
        // Not supported. Very low priority, it's super fast as it is.
        // ( kNThreads.c_str(), po::value<unsigned>(),
        //   "Number of threads (trimmed to number of cpus in the machine) " )

        // Not supported yet
        // ( kRulesToWeightsLatticeFilterbyAlilats.c_str(),
        //   "Filter the flower lattice with the vocabulary of the alignment lattices" )
        ( kRulesToWeightsLoadGrammar.c_str(), po::value<std::string>(),
          "Load a synchronous context-free grammar file" )
        ( kRulesToWeightsLoadalilats.c_str() ,
          po::value<std::string>(), "Load hifst sparse weight translation lattice" )
        ( kRulesToWeightsNumberOfLanguageModels.c_str()
          , po::value<unsigned>()->default_value ( 1 )
          , "Number of language models" )
        ;
  }
  desc.add_options()
      ( kRulesToWeightsLatticeStore.c_str() ,
        po::value<std::string>()->default_value ( "" ),
        "Store the fst (tropical tuple sparse weight) containing a vector of features per arc " )
      ;
}


inline void checkRules2Weightptions (po::variables_map *vm) {
  using namespace HifstConstants;
  if ( vm->count ( kRulesToWeightsLoadGrammar.c_str() ) ) {
    LDEBUG ( kRulesToWeightsLoadGrammar <<
             ( *vm ) [kRulesToWeightsLoadGrammar.c_str()].as<std::string>() );
  } else {
    LERROR ( "parameter " << kRulesToWeightsLoadGrammar << " not defined" );
    exit ( EXIT_FAILURE );
  }
};

}
}  // end namespaces
