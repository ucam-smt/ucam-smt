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

#ifndef PATTERNTOINSTANCESTASK_HPP
#define PATTERNTOINSTANCESTASK_HPP

/**
 * \file
 * \brief Contains patterns to instance-patterns implementation.
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 * \remark This file has been reviewed/modified by:
 */

namespace ucam {
namespace hifst {

/**
 * \brief Converts patterns to instanced patterns.
 * \remark Given a set of grammar-specific source patterns and a source sentence, generate instances of these patterns.
 * Example, given pattern w_X_w and sentence "1 3 4 5 2", generate 1_X_2, 1_3_X_2 , 1_3_4_X_2 , 1_3_X_5_2 ,...
 */

template <class Data>
class PatternsToInstancesTask: public ucam::util::TaskInterface<Data> {

  //Private variables are shown here. Private methods go after public methods
 private:
  ///Maximum pattern/rule span
  unsigned maxspan_;

  ///Maximum gap (non-terminal) span
  unsigned gapmaxspan_;
  ///filename with wildcards.
  ucam::util::IntegerPatternAddress instancefile_;

 public:
  /**
   * \brief Constructor
   * \param rg: RegistryPO object with configfile/command-line params parsed and ready to use.
   */
  PatternsToInstancesTask ( const ucam::util::RegistryPO& rg ) :
    maxspan_ ( rg.get<unsigned> ( "patternstoinstances.maxspan" ) ),
    gapmaxspan_ ( rg.get<unsigned> ( "patternstoinstances.gapmaxspan" ) ),
    instancefile_ ( rg.get<std::string> ( "patternstoinstances.store" ) ) {
    LDEBUG ( "Ready!" );
  };

  /**
   * \brief Runs this task and modifies Data object inserting the instanced sentence-specific source patterns we were looking for.
   * \param d: Data object.
   */

  bool run ( Data& d ) {
    LINFO ( "instancing " << d.grammar->patterns.size() <<
            " patterns over this sentence:" << d.sentence );
    d.stats->setTimeStart ("instantiate-patterns");
    instantiatePatternsHash ( d );
    d.stats->setTimeEnd ("instantiate-patterns");
    writeHashToFile ( d );
    LINFO ( "Finished!" );
    return false;
  };

  ///Destructor
  ~PatternsToInstancesTask() {};

#ifndef TESTING
 private:
#endif

  /**
   * \brief Instantiates patterns and stores position/span.
   * \param d: Templated Data object.
   */

  void instantiatePatternsHash ( Data& d ) {
    d.hpinstances.clear();
    LINFO ( "maxspan_=" << maxspan_ << ",gapmaxspan=" << gapmaxspan_ );
    std::vector<std::string> ss;
    boost::algorithm::split ( ss, d.sentence, boost::algorithm::is_any_of ( " " ) );
    const unordered_set<std::string>& patterns = d.grammar->patterns;
    for ( unordered_set<std::string>::const_iterator itx = patterns.begin();
          itx != patterns.end(); ++itx ) { /// for each grammar-specific pattern.
      LDEBUG ( "pattern:" << *itx );
      std::vector<std::string> spattern;
      boost::algorithm::split ( spattern, *itx, boost::algorithm::is_any_of ( "_" ) );
      for ( unsigned j = 0; j < ss.size(); ++j ) { // for each word in the sentence
        std::vector< std::vector<std::string> > pinstances;
        LDEBUG ( "starting word:" << ss[j] );
        //Map each pattern into words.
        //If there are gaps, then expand them from 1 to given threshold gapmaxspan
        if ( spattern.size() <= maxspan_ && j + spattern.size() - 1 < ss.size() ) {
          std::vector<std::string> empty;
          pinstances.push_back ( empty ); //add empty one.
          //Create all instances that apply to this particular pattern.
          buildNextElementFromPattern ( spattern, ss, pinstances, j, 0 );
        }
        for ( unsigned k = 0; k < pinstances.size(); ++k )  {
          LDEBUG ( "pattern:" << *itx << ":" << "Inserting in " <<
                   boost::algorithm::join ( pinstances[k],
                                            "_" ) << "values=(" << j << "," << spattern.size() - 1 );
          d.hpinstances[boost::algorithm::join ( pinstances[k],
                                                 "_" )].push_back ( pair<unsigned, unsigned> ( j, spattern.size() - 1 ) );
        }
      }
    }
  }

  /**
   * \brief Recursive function used to instantiate patterns on the source sentence. Meant to be called from instantiatePatterns.
   * \param spattern              Pattern being scrutinized
   * \param ss                    Sentence
   * \param pinstances            Pattern instances found
   * \param ps                    Sentence index
   * \param pp                    Pattern  index
   * \param gaphistory            gap size we are currently working with
   */

  void buildNextElementFromPattern ( std::vector<std::string>& spattern,
                                     std::vector<std::string>& ss,
                                     std::vector< std::vector<std::string> >& pinstances,
                                     unsigned ps,
                                     unsigned pp,
                                     unsigned gaphistory = 0 ) {
    LDEBUG ( "startingword:" << ss[ps] << ",thisword:" <<   ss[ps + pp +
             gaphistory ] << ",thiselement:" << spattern[pp] << ",ps=" << ps << ",pp=" << pp
             << ",spatternsize=" << spattern.size() << ",gaphistory=" << gaphistory );
    if ( spattern[pp] == "w" ) {
      pinstances[pinstances.size() - 1].push_back ( ucam::util::toString (
            ss[ps + pp + gaphistory] ) );
      if ( ( pp + 1 < spattern.size() )
           && ( ps + spattern.size() + gaphistory  <= ss.size() ) )
        buildNextElementFromPattern ( spattern, ss, pinstances, ps, pp + 1,
                                      gaphistory );
    } else if ( spattern[pp] == "X" ) {
      LDEBUG ( "X,with gapmaxspan=" << gapmaxspan_ );
      pinstances[pinstances.size() - 1].push_back ( "X" );
      std::vector<std::string> replicate = pinstances[pinstances.size() - 1];
      for ( unsigned k = 1;
            ( k <= gapmaxspan_ )
            && ( pp + 1 < spattern.size() )
            && ( ps + spattern.size() - 1 + gaphistory + k - 1 < ss.size() )
            && ( spattern.size() + gaphistory + k - 1 <= maxspan_ );
            ++k ) {
        LDEBUG ( "GAPSPAN=" << k );
        if ( k > 1 ) pinstances.push_back (
            replicate ); //clone previous one and run recursively.
        buildNextElementFromPattern ( spattern, ss, pinstances, ps, pp + 1,
                                      gaphistory + k - 1 );
      }
    } else {
      ///Bad news if you get here... Expliciting failed condition to provide enough user information...
      USER_CHECK ( spattern[pp] == "X" || spattern[pp] == "w", "Incorrect pattern!" );
    }
  };

  /**
   * \brief Writes instances patterns to file.
   * \param d: Templated data object. Takes instances and sentence idx.
   */

  void writeHashToFile ( Data& d ) {
    std::string file = instancefile_ ( d.sidx );
    if ( file != "" ) {
      LINFO ( "file to output:" << file );
      ucam::util::oszfstream o ( file + ".hash" );
      for ( unordered_map<std::string, std::vector <pair <unsigned, unsigned> > >::iterator
            itx = d.hpinstances.begin(); itx != d.hpinstances.end(); ++itx ) {
        o << itx->first << ":" ;
        for ( unsigned k = 0; k < itx->second.size(); ++k )
          o << itx->second[k].first << "," << itx->second[k].second << ";";
        o << endl;
      }
      o.close();
    }
  };

  ///Private constructor. For testing purposes only.
  PatternsToInstancesTask ( unsigned maxspan_
                            , unsigned gapmaxspan_
                            , const std::string& instancefile_ ) :
    maxspan_ ( maxspan_ ),
    gapmaxspan_ ( gapmaxspan_ ),
    instancefile_ ( instancefile_ ) {
    LDEBUG ( "Ready!" );
  };

  ZDISALLOW_COPY_AND_ASSIGN ( PatternsToInstancesTask );
};

}
}   // End namespaces

#endif
