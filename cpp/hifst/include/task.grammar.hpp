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

#ifndef RULEFILETASK_HPP
#define RULEFILETASK_HPP

#include "task.grammar.nonterminalhierarchy.hpp"

/** \file hifst/include/task.grammar.hpp
 *    \brief Describes class GrammarTask
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 *\brief Task class that loads a grammar into memory.
 *
 * It provides methods to take as input a [file], and read the contents, sort and populate GrammarData.
 * Optionally it can also store grammar-specific patterns.
 * \remark Inherits properties from TaskInterface and is templated over a data class, in which relevant data is stored.
 */

template <class Data>
class GrammarTask: public ucam::util::TaskInterface<Data> {

  //Private variables are shown here. Private methods go after public methods
 private:

  /// Expandable strings for sentence-specific  grammar files and pattern files.
  ucam::util::IntegerPatternAddress grammarfile_, patternfile_;
  ///Previous grammar file.
  std::string previous_;
  PatternCompareTool pct_;
  /// Structure containing the grammar
  GrammarData gd_;
  uint pos_;
  /// Priority queue used for pattern-based index sorting.
  std::priority_queue<posindex, std::vector<posindex>, PosIndexCompare> *vpq_;

  ///Determines non-terminal hierarchy automatically from the grammar file...
  NonTerminalHierarchy nth_;

  std::vector<float> grammarscales_;
  std::string ntorderfile_;

 public:
  /**
   *\brief Constructor
   *
   * \param rg Pointer to a Registry object initialized with boost::program_options parsed variables.
   */

  GrammarTask ( ucam::util::RegistryPO const& rg
                , std::string const& featureweightskey = HifstConstants::kGrammarFeatureweights
                    , unsigned featureoffset = 0) :
    previous_ ( "" ),
    grammarfile_ ( rg.get<std::string> ( HifstConstants::kGrammarLoad ) ),
    patternfile_ ( rg.get<std::string> ( HifstConstants::kGrammarStorepatterns ) ) ,
    ntorderfile_ (rg.get<std::string> ( HifstConstants::kGrammarStorentorder) ),
    grammarscales_ ( ucam::util::ParseParamString<float> ( rg.get<std::string>
                     ( featureweightskey ) ) ) {
    gd_.ct = &pct_;
    if (featureoffset ) {
      std::vector<float> aux (grammarscales_.size() - featureoffset);
      std::copy (grammarscales_.begin() + featureoffset, grammarscales_.end(),
                 aux.begin() );
      grammarscales_ = aux;
    }
    USER_CHECK ( grammarscales_.size(),
                 "0 feature weights. So the grammar is not a probabilistic model? Not my cup of tea." );
  };

  /**
   *\brief Constructor used for unit testing
   *
   * \param grammarfilekey       Registry key accessing file name to load the grammar from.
   * \param patternfilekey       Registry key accessing  file name to dump the patterns.
   */

  GrammarTask ( const std::string& grammarfilekey = HifstConstants::kGrammarLoad,
                const std::string& patternfilekey = HifstConstants::kGrammarStorepatterns ) :
    previous_ ( "" ),
    grammarfile_ ( grammarfilekey ),
    patternfile_ ( patternfilekey ) ,
    grammarscales_ ( ucam::util::ParseParamString<float> ( "1" ) ) {
  };

  /**
   *\brief Returns GrammarData
   *\return Pointer to GrammarData class.
   */
  inline GrammarData *getGrammarData() {
    return &gd_;
  };

  /**
   *\brief ucam::util::TaskInterface mandatory method implementation.
   * This method loads the hierarchical grammar, stores patterns,
   * finds non-terminal hierarchy and delivers pointer to data object, for other tasks to use the grammar
   * \param d          Data Object
   */

  bool run ( Data& d ) {
    std::string thisgrammarfile = grammarfile_ ( d.sidx );
    if ( thisgrammarfile != previous_ ) {
      FORCELINFO ( "Loading hierarchical grammar: " << thisgrammarfile );
      USER_CHECK ( ucam::util::fileExists ( thisgrammarfile ),
                   "This grammar does not exist" );
      d.stats->setTimeStart ( "load-grammar-patterns" );
      load ( thisgrammarfile );
      d.stats->setTimeEnd ( "load-grammar-patterns" );
      std::string patternfile = patternfile_ ( d.sidx );
      if ( patternfile != "" ) {
        ucam::util::oszfstream o ( patternfile );
        for ( std::unordered_set<std::string>::iterator itx = gd_.patterns.begin();
              itx != gd_.patterns.end(); ++itx ) o << *itx << std::endl;
        o.close();
      }
      previous_ = thisgrammarfile;
    } else {
      LINFO ( "Skipping grammar loading..." );
    }
    d.grammar = &gd_;
    return false;
  };

  /**
   *\brief Loads rules from a grammar file.
   *
   * \param file Full pathname to the grammar file.
   * \return void
   */

  inline void load ( const std::string& file ) {
    load_init();
    LINFO ( "=> Loading..." << file );
    ucam::util::readtextfile<GrammarTask> ( file, *this );
    load_sort();
    LINFO ( "Done! ****" );
    generate_ntorder();
  };

  /**
   *\brief Loads rules from a stringstream
   *
   * \param s stream with rules.
   * \return void
   */

  inline void load ( std::stringstream& s ) {
    load_init();
    std::string myline;
    while ( getline ( s, myline ) ) {
      parse ( myline );
    }
    load_sort();
    LINFO ( "Done!" );
    generate_ntorder();
  };

  virtual ~GrammarTask() {};

 private:

  /**
   * \brief Generates non-terminal hierarchical using the functor NonTerminalHierarchy
   *
   */
  void generate_ntorder() {
    std::string ntorder;
    nth_ ( ntorder );
    LINFO ( "ntorder=" << ntorder );
    std::vector<std::string> aux;
    boost::algorithm::split ( aux, ntorder, boost::algorithm::is_any_of ( " ," ) );
    for ( uint k = 0; k < aux.size(); ++k ) {
      gd_.vcat[k + 1] = aux[k]; //Note that mapped indices always start from 1
      gd_.categories[aux[k]] = k + 1;
    }
    if (ntorderfile_ != "") {
      ucam::util::oszfstream o ( ntorderfile_ );
      for ( uint k = 0; k < gd_.vcat.size(); ++k )
        o << gd_.vcat[k + 1] << "\t" << k + 1 << std::endl;
    }
  }

  /**
   *\brief Init variables for grammar file loading
   * \return void
   */

  inline void load_init() {
    pos_ = 0;
    gd_.reset();
    gd_.ct = &pct_;
    vpq_ = new
    std::priority_queue<posindex, std::vector<posindex>, PosIndexCompare>
    ( PosIndexCompare ( &gd_.filecontents, gd_.ct ) );
  };

  /**
   *\brief Sort indices using a priority queue and pattern sorting.
   * \return void
   */

  inline void load_sort() {
    LINFO ( "Sorting indices..." );
    uint newidx = 0;
    gd_.sizeofvpos = vpq_->size();
    gd_.vpos = new
    posindex[gd_.sizeofvpos]; //peak memory footprint here, we could avoid this by enforcing sorted grammar input (although it would have to meet the same pattern sorting criterion...)
    LINFO ( gd_.sizeofvpos << " indices" );
    while ( !vpq_->empty() ) {
      gd_.vpos[newidx++] = vpq_->top();
      vpq_->pop();
      LDEBUG2 ( gd_.getRule ( newidx - 1 ) << " at " << gd_.vpos[newidx - 1].order );
    }
    delete vpq_;
  };

  /**
   *\brief  Parses a rule, updates the GrammarData structure and the priority queue
   * \param line synchronous rule to parse.
   * \return void
   */

  __always_inline void parse ( std::string& line ) {
    using namespace std;
    using namespace ucam::util;

    boost::algorithm::trim ( line );
    if ( line == "" ) return;
    size_t pos1 = line.find_first_of ( " " ); // src
    size_t pos2 = line.find_first_of ( " ", pos1 + 1 ); // trg
    size_t pos3 = line.find_first_of ( " ", pos2 + 1 ); // weight

    if (pos3 == std::string::npos) {
      LERROR("Grammar not valid. At least one weight is needed: \n=>\t" << line);
      exit(EXIT_FAILURE);
    }
    size_t pos4 = line.find_first_of ( "\t"); // optional alignments
    if (pos4 == std::string::npos) pos4 = line.size();
    LDEBUG("pos1=" << pos1 << ",pos2=" << pos2 << ",pos3=" << pos3 << ",pos4=" << pos4);

    vector<float> weights;
    ParseParamString<float> ( line, weights, pos3 + 1 , pos4 - pos3 - 1 );
    string sweight = toString<float>
        ( dotproduct (weights, grammarscales_ ), numeric_limits<unsigned>::max() );
    trim_trailing_zeros ( sweight );
    line = ( pos4 <line.size() )
        ? line.substr ( 0, pos3 + 1 ) + sweight + line.substr(pos4)
        : line.substr ( 0, pos3 + 1 ) + sweight;


    LDEBUG("Adding line=[" << line  << "]");
    gd_.filecontents += line + '\n';
    posindex pi;
    bool waitingfornextfield = false;
    unsigned cf = 2; //Second field
    char previous = ' ';
    for ( unsigned k = 0; k < line.size(); ++k ) {
      if ( previous == ' ' && line[k] != ' ' ) --cf;
      if ( !cf ) {
        pi.o = k;
        break;
      }
      previous = line[k];
    }
    pi.p = pos_ + pi.o;
    string pattern;
    bool word = false;
    bool nt = false;
    for ( unsigned k = pi.o; k < line.size(); ++k ) {
      if ( line[k] == ' ' ) break;
      if ( line[k] >= '0' && line[k] <= '9' ) {
        if ( !word && !nt ) {
          pattern += 'w';
          word = true;
          nt = false;
        }
      } else if ( line[k] >= 'A' && line[k] <= 'Z' ) {
        if ( !nt ) {
          pattern += 'X';
          nt = true;
          word = false;
        }
      } else {
        pattern += line[k];
        nt = word = false;
      }
    }
    if ( gd_.patterns.find ( pattern ) == gd_.patterns.end() ) {
      gd_.patterns.insert ( pattern );
    }
    pi.order = vpq_->size();
    vpq_->push ( pi );
    pos_ += line.size() + 1;
    LDEBUG2 ( "reading rule " << line << ", at line " << pi.order << ", pattern=" <<
              pattern );
    if ( pattern == "X" ) {
      LINFO ( "Identity rule detected:" << line << "===" );
      nth_.insertIdentityRule ( line );
    } else {
      nth_.insertLHS ( line.substr ( 0, pi.o - 1 ) );
    }
  };

  ///Friendship with readtextfile function.
  template <typename FM>
  friend inline void ucam::util::readtextfile ( const std::string& filename,
      FM& fm );

  ZDISALLOW_COPY_AND_ASSIGN ( GrammarTask );

};

}
}  // end namespaces
#endif
