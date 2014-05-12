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
 *    \brief To initialize boost parameter options
 * \date 21-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace util {

namespace po = boost::program_options;

inline void initCommonApplylmOptions (po::options_description& desc) {
  desc.add_options()
  ( HifstConstants::kLmFeatureweights.c_str(),
    po::value<std::string>()->default_value ( "1.0" ),
    "Scaling factors applied to the language models (separated by commas). Assumed as 1 for all language models if not specified " )
  ( HifstConstants::kLmWordPenalty.c_str(),
    po::value<std::string>()->default_value ( "0.0" ),
    "Word penalty applied along the language models (separated by commas). Assumed as 0 if not specified " )
  ( HifstConstants::kLmLoad.c_str(),
    po::value<std::string>()->default_value ( "" ),
    "Load one or more language models (separated by commas)" )
  ( HifstConstants::kLmWordmap.c_str(),
    po::value<std::string>()->default_value ( "" ),
    "Use external integer-map file for the language model" )
  ( HifstConstants::kLmLogTen.c_str(),
    "Does not convert lm scores to natural log" )
  ;
}

inline void checkApplyLmOptions (po::variables_map *vm) {
  if ( vm->count ( HifstConstants::kLmLoad.c_str() ) ) {
    LDEBUG ( HifstConstants::kLmLoad << "=" <<
             ( *vm ) [HifstConstants::kLmLoad.c_str()].as<std::string>() );
  } else {
    LERROR ( "Language model file not defined" );
    exit ( EXIT_FAILURE );
  }
}

}
} // end namespaces
