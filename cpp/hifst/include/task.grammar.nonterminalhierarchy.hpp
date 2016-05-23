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

#ifndef TASK_GRAMMAR_NONTERMINALHIERARCHY
#define TASK_GRAMMAR_NONTERMINALHIERARCHY

/**
 * \file
 * \brief this class decides automatically the hierarchy of non-terminals
 * \date 22-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief This is a functor with additional methods to include relevant rules
 * (i.e. identify SCFG rules, S -> X X, X -> V V  ) and determine the hierarchy
 * of non-terminals within the grammar.
 */
class NonTerminalHierarchy {
 private:
  ///Contains identity rules
  std::unordered_set<std::string> h_;
  ///Contains set of non-terminals
  std::unordered_set<std::string> nt_;

  ///Determines whether s_ has been found or not.
  bool s_;

 public:
  ///Constructor
  NonTerminalHierarchy() {};
  ///Method to store identity rules, i.e. S -> X X , etc
  inline void insertIdentityRule ( const std::string& identityrule ) {
    h_.insert ( identityrule );
  };

  ///Insert LHS non-terminals. Use this method to make sure that all non-terminals
  ///(even those that are not used in identity rules) are considered.
  inline void insertLHS ( const std::string& nt ) {
    nt_.insert ( nt );
  };

  /**
  * \brief Determines hierarchical list of non-terminals and flags whether there has been a cycle or not
  * Example: For a SCFG rule S -> X X, hierarchy is S,X. If we add X -> S S, then we have a cycle.
  * For more examples see unit test
  * \param ntlist On completion, it will store the actual list of hierachically sorted non-terminals
  */

  bool operator() ( std::string& ntlist ) {
    s_ = false;
    ntlist = "";
    std::unordered_set<std::string> haux = h_;
    std::unordered_set<std::string> nt = nt_;
    populate_nt ( nt );
    if ( !nt.size() ) return true;
    uint count = 0;
    uint previoussize;
    do {
      previoussize = haux.size();
      LINFO ( "round " << ++count << ": haux.size=" << previoussize );
      ntlist += identify_highest_nt ( haux, nt );
      if ( !s_ ) {
        LERROR ( "Incorrect grammar? S is expected to have the highest hierarchy!" );
        boost::algorithm::trim ( ntlist );
        ucam::util::find_and_replace ( ntlist, " ", "," );
        return false;
      }
    } while ( haux.size() < previoussize );
    boost::algorithm::trim ( ntlist );
    if ( haux.size() ) {
      LINFO ( "Conflict!" );
      LINFO ( "Partial list of ordered non-terminals:" << ntlist );
      LINFO ( "Rules containing a cycle:" );
      for ( std::unordered_set<std::string>::iterator itx = haux.begin();
            itx != haux.end(); ++itx )
        LINFO ( "===" << *itx );
      return false;
    }
    ntlist = "S " + ntlist;
    ucam::util::find_and_replace ( ntlist, " ", "," );
    LINFO ( "DONE!" );
    return true;
  }

 private:

  ///Given a set of rules, identifies non-terminals and populates an unordered_set with them.
  void populate_nt ( std::unordered_set<std::string>& nt ) {
    for ( std::unordered_set<std::string>::iterator itx = h_.begin(); itx != h_.end();
          ++itx ) {
      std::vector<std::string> aux;
      boost::algorithm::split ( aux, *itx, boost::algorithm::is_any_of ( " " ) );
      getFilteredNonTerminal ( aux[1] );
      nt.insert ( aux[0] );
      nt.insert ( aux[1] );
    }
    LINFO ( "Number of non-terminals=" << nt.size() );
  }

  ///Identifies (partial list of)  highest order non-terminals in he hierarchy,
  /// given the identity rules and the set of non-terminals. Rules and non-terminals
  ///that have been accounted for will be deleted. In order to identify the complest list
  ///one or more consecutive calls to this method required
  std::string identify_highest_nt ( std::unordered_set<std::string>& h,
                                    std::unordered_set<std::string>& nt ) {
    std::map<std::string, uint> ntorder;
    int order = 0;
    for ( std::unordered_set<std::string>::iterator itx = nt.begin();
          itx != nt.end(); ++itx ) {
      ntorder[*itx] = 0;
      LINFO ( "nt:" << *itx );
    }
    for ( std::unordered_set<std::string>::iterator itx = h.begin();
          itx != h.end(); ++itx ) {
      std::vector<std::string> aux;
      boost::algorithm::split ( aux, *itx, boost::algorithm::is_any_of ( " " ) );
      getFilteredNonTerminal ( aux[1] );
      ntorder[aux[1]]++;
    }
    LINFO ( "Extract highest nt" );
    std::string ntstring;
    std::unordered_set<std::string> nttodelete;
    for ( std::map<std::string, uint>::iterator itx = ntorder.begin();
          itx != ntorder.end(); ++itx ) {
      if ( !itx->second ) {
        if ( itx->first != "S" ) {
          LINFO ( "Found " << itx->first );
          ntstring += " " + itx->first;
        } else {
          LINFO ( "Found category S" );
          s_ = true; //This one should be encountered first call and must hold highest hierarchy. Will be prepended at the end
        }
        nt.erase ( itx->first );
        nttodelete.insert ( itx->first );
      }
    }
    std::unordered_set<std::string> nttodelete2;
    for ( std::unordered_set<std::string>::iterator itx = h.begin(); itx != h.end();
          ++itx ) {
      std::vector<std::string> aux;
      boost::algorithm::split ( aux, *itx, boost::algorithm::is_any_of ( " " ) );
      if ( nttodelete.find ( aux[0] ) != nttodelete.end() ) {
        nttodelete2.insert ( *itx );
      }
    }
    for ( std::unordered_set<std::string>::iterator itx = nttodelete2.begin();
          itx != nttodelete2.end(); ++itx ) {
      h.erase ( *itx );
    }
    LINFO ( "ntstring=" << ntstring << ", HSIZE=" << h.size() << "NTSIZE=" <<
            nt.size() );
    return ntstring;
  }

  ZDISALLOW_COPY_AND_ASSIGN ( NonTerminalHierarchy );

};

}
} // end namespaces

#endif
