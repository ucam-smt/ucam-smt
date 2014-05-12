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

#include <main.createssgrammar.init_param_options_common.hpp>
#include <main.applylm.init_param_options_common.hpp>

/** \file
 * \brief To initialize boost parameter options for hifst tool
 * \date 8-8-2012
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
 * \param argc   number of command-line options, as generated for the main function
 * \param argv   standard command-line options, as generated for the main function
 * \param vm     boost variable containing all parsed options.
 * \return void
 */

inline void init_param_options ( int argc, const char* argv[],
                                 po::variables_map *vm ) {
  try {
    po::options_description desc ( "Command-line/configuration file options" );
    initAllCreateSSGrammarOptions (desc); // All createssgrammar options are used
    initCommonApplylmOptions (desc); // Add generic language model options
    desc.add_options()
    ( HifstConstants::kServerEnable.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Run in server mode (yes|no)" )
    ( HifstConstants::kServerPort.c_str()
      , po::value<short>()->default_value ( 1209 )
      , "Server port" )
    ( HifstConstants::kTargetStore.c_str()
      , po::value<std::string>()->default_value ( "-" )
      , "Source text file -- this option is ignored in server mode" )
    ( HifstConstants::kFeatureweights.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Feature weights applied in hifst. This is a comma-separated sequence "
      "of language model(s) and grammar feature weights.\n"
      "IMPORTANT: If this option is not empty string, then it will override "
      "any values in lm.featureweights and grammar.featureweights"
    )
    ( HifstConstants::kReferencefilterLoad.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Reference lattice to filter the translation" )
    ( HifstConstants::kReferencefilterWrite.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Write reference lattice" )
    ( HifstConstants::kReferencefilterSubstring.c_str()
      , po::value<std::string>()->default_value ( "yes" )
      , "Substring the reference lattice (yes|no)" )
    ( HifstConstants::kReferencefilterPrunereferenceweight.c_str()
      , po::value<float>()->default_value ( std::numeric_limits<float>::max() )
      , "Likelihood beam to prune the reference lattice" )
    ( HifstConstants::kReferencefilterPrunereferenceshortestpath.c_str()
      , po::value<unsigned>()->default_value (
        std::numeric_limits<unsigned>::max() ) )
    ( HifstConstants::kCykparserHrmaxheight.c_str()
      , po::value<unsigned>()->default_value ( 10 )
      , "Default maximum span for hierarchical rules" )
    ( HifstConstants::kCykparserHmax.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Maximum span for individual non-terminals, constrained to hrmaxheight : e.g. X,10,V,6" )
    ( HifstConstants::kCykparserHmin.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Minimum span for individual non-terminals, constrained to hrmaxheight: e.g. X,3,V,2" )
    ( HifstConstants::kCykparserNtexceptionsmaxspan.c_str()
      , po::value<std::string>()->default_value ( "S" )
      , "List of non-terminals not affected by cykparser.hrmaxheight. S should always be in this list!" )
    ( HifstConstants::kHifstLatticeStore.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Store hifst translation lattice" )
    ( HifstConstants::kHifstAlilatsmode.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Include derivations in the left side of transducers (yes|no)" )
    ( HifstConstants::kHifstUsepdt.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Run hifst using pdt representation, aka hipdt (yes|no)" )
    ( HifstConstants::kHifstRtnopt.c_str()
      , po::value<std::string>()->default_value ( "yes" )
      , " Use openfst rtn optimizations (yes|no)" )
    ( HifstConstants::kHifstOptimizecells.c_str()
      , po::value<std::string>()->default_value ( "yes" )
      , "Determinize/minimize any FSA component of the RTN (yes|no)"  )
    ( HifstConstants::kHifstReplacefstbyarcNonterminals.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Determine which cell fsts are always replaced by single arc according to its non-terminals, e.g: replacefstbyarc=X,V" )
    ( HifstConstants::kHifstReplacefstbyarcNumstates.c_str()
      , po::value<unsigned>()->default_value ( 4 )
      , "Determine the minimum number of states that triggers replacement by arc." )
    ( HifstConstants::kHifstReplacefstbyarcExceptions.c_str()
      , po::value<std::string>()->default_value ( "S" )
      , "Categories that will definitely not be replaced (takes over replacefstbyarc and replacefstbyarc.numstates)" )
    ( HifstConstants::kHifstLocalpruneEnable.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Apply local pruning strategy based con cyk cells and number of states (yes|no)" )
    ( HifstConstants::kHifstLocalpruneLmLoad.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Load one or more language model files: (gzipped) arpa format or kenlm binary format (uses memory mapping); separated by commas" )
    ( HifstConstants::kHifstLocalpruneLmFeatureweights.c_str()
      , po::value<std::string>()->default_value ( "1.0" )
      , "Scaling factor(s) applied to the language model: arpa_weight * -log(10) * gscale. Scales separated by commas." )
    ( HifstConstants::kHifstLocalpruneLmWordpenalty.c_str()
      , po::value<std::string>()->default_value ( "0.0" )
      , "Word penalty applied along the language models (separated by commas). Assumed as 0 if not specified " )
    ( HifstConstants::kHifstLocalpruneNumstates.c_str()
      , po::value<unsigned>()->default_value ( 10000 )
      , "Maximum number of states threshold after cell pruning an FSA, If beneath the threshold, determinization/minimization is applied to pruned lattice. Also applicable in alignment mode when filtering against substring acceptor. ")
    ( HifstConstants::kHifstLocalpruneConditions.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Local pruning conditions. These are sequences of 4-tuples separated by commas: category,span,number_of_states,weight. The three first are actual thresholds that trigger local pruning, whereas the weight is the likelihood beam for pruning, IF a language model has been applied." )
    ( HifstConstants::kHifstPrune.c_str()
      , po::value<float>()->default_value ( std::numeric_limits<float>::max() )
      , "Likelihood beam to prune the translation lattice. Only applied IF a language model is available." )
    ( HifstConstants::kHifstWritertn.c_str()
      , po::value<std::string>()->default_value ( "")
      , "Write the rtn to disk -- long list of FSAs. Use %%rtn_label%% and ? to format file names appropriately, e.g. --hifst.writertn=rtn/?/%%rtn_label%%.fst" )
    ( HifstConstants::kRecaserLmLoad.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Language model for recasing" )
    ( HifstConstants::kRecaserLmFeatureweight.c_str()
      , po::value<std::string>()->default_value ( "1.0" )
      , "Scaling factor applied to the language model" )
    ( HifstConstants::kRecaserUnimapLoad.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "unigram transduction model  " )
    ( HifstConstants::kRecaserUnimapWeight.c_str()
      , po::value<float>()->default_value ( 1.0f )
      , "Scaling factors applied to the unigram model " )
    ( HifstConstants::kRecaserPrune.c_str()
      , po::value<std::string>()->default_value ( "byshortestpath,1" )
      , "Choose between byshortestpath,numpaths or byweight,weight" )
    ( HifstConstants::kRecaserOutput.c_str()
      , po::value<std::string>()->default_value ("")
      , "Output true cased lattice" )
    ( HifstConstants::kPostproWordmapLoad.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Load a reverse integer mapping file so the decoder can map integers to target words" )
    ( HifstConstants::kPostproDetokenizeEnable.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Detokenize translated 1best (yes|no) -- NOT IMPLEMENTED!" )
    ( HifstConstants::kPostproDetokenizeLanguage.c_str()
      , po::value<std::string>()->default_value ( "" ), "NOT IMPLEMENTED" )
    ( HifstConstants::kPostproCapitalizefirstwordEnable.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Capitalize first word (yes|no). Only applies if previously mapped back to words (postpro.wordmap.load)" )
    ( HifstConstants::kStatsHifstWrite.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Dump hifst-specific stats (cyk, local pruning, etc)" )
    ( HifstConstants::kStatsHifstCykgridEnable.c_str()
      , po::value<std::string>()->default_value ( "no" )
      , "Write cyk/rtn stats to the file (yes|no)" )
    ( HifstConstants::kStatsHifstCykgridCellwidth.c_str()
      , po::value<unsigned>()->default_value ( 30 )
      , "Width of the printed cyk cell" )
    ( HifstConstants::kStatsWrite.c_str()
      , po::value<std::string>()->default_value ( "" )
      , "Dump general stats (speed and general messages)" )
    ;
    parseOptionsGeneric (desc, vm, argc, argv);
    checkCreateSSGrammarOptions (vm);
    if ( (*vm) [HifstConstants::kPatternstoinstancesMaxspan.c_str() ].as<unsigned>()
         < (*vm) [ HifstConstants::kCykparserHrmaxheight.c_str()].as<unsigned>() ) {
      LERROR ( HifstConstants::kPatternstoinstancesMaxspan <<
               " cannot be smaller than " << HifstConstants::kCykparserHrmaxheight);
      exit (EXIT_FAILURE );
    }
    if (  (*vm) [HifstConstants::kFeatureweights.c_str()].as<std::string>() != ""
          && ( (*vm) [HifstConstants::kLmFeatureweights.c_str()].as<std::string>() != ""
               || (*vm) [HifstConstants::kGrammarFeatureweights.c_str()].as<std::string>() !=
               "" ) ) {
      LWARN ("Program option featureweights OVERRIDES grammar.featureweights and lm.featureweights!!");
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
} // end namespaces
