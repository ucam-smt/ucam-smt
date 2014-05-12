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

/**
 * \file
 * \brief Contains cyk parser implementation.
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef CYKPARSER_HPP
#define CYKPARSER_HPP

#define CYK_RETURN_FAILURE 0

namespace ucam {
namespace hifst {

///Implements cyk+ parser
template <class Data>
class CYKParserTask : public ucam::util::TaskInterface<Data> {

  //Private variables are shown here. Private methods go after public methods
 private:

  unsigned finalresult_;

  ///hiero rule max height (S is hardwired as an exception)
  unsigned hrmh_;

  ///non-terminal maximum span.
  unordered_map <std::string, unsigned> hmax_;
  ///non-terminal minimum span
  unordered_map <std::string, unsigned> hmin_;

  ///Number of non-terminals
  unsigned nnt_;

  /// cyk data structure.
  CYKdata cykdata_;

  ///Pointer to a data structure.
  Data *d_;

 public:

  /**
   * \brief Constructor
   * \param rg: registry object.
   */
  CYKParserTask ( const ucam::util::RegistryPO& rg ) :
    d_ ( NULL ),
    finalresult_ ( CYK_RETURN_FAILURE ),
    hrmh_ ( rg.get<unsigned> ( HifstConstants::kCykparserHrmaxheight ) ),
    hmax_ ( rg.getPairMappedStringUInt ( HifstConstants::kCykparserHmax ) ),
    hmin_ ( rg.getPairMappedStringUInt ( HifstConstants::kCykparserHmin ) ) {
    cykdata_.nt_exceptions_maxspan = rg.getSetString (
                                       HifstConstants::kCykparserNtexceptionsmaxspan);
    LDEBUG ( "Constructor done!" );
  };

  /**
   * \brief      Runs the parsing algorithm.
   * \param d    Contains sentence to parse, grammar, sentence-specific rule indices, etc.
   * \returns    False (not last task to run) or true (last task to run)
   */

  bool run ( Data& d ) {
    d.cykdata = &cykdata_;
    d_ = &d;
    const GrammarData& gd = *d_->grammar;
    cykparser_rulebpcoordinates_t coord;
    cykdata_.freeMemory();
    grammar_categories_t& categories = cykdata_.categories;
    categories = gd.categories;
    grammar_inversecategories_t& vcat = cykdata_.vcat;
    vcat = gd.vcat;
    unsigned& nnt = cykdata_.nnt;
    nnt = categories.size();
    cykparser_sentence_t& sentence = cykdata_.sentence;
    sentence.clear();
    ///\todo S is hardwired as the highest order non-terminal.
    /// We could change this so that it automatically accepts the non-terminal with the highest hierarchy (or any non-terminal), or one specified by the user.
    USER_CHECK ( cykdata_.categories["S"] == 1, "Mandatory to use S as first guy" );
    USER_CHECK ( cykdata_.vcat[1] == "S",
                 "Mandatory to have an S as the first non-terminal guy" );
    std::vector<std::string> aux;
    boost::algorithm::split ( aux, d.sentence,
                              boost::algorithm::is_any_of ( " " ) );
    for ( unsigned k = 0; k < aux.size(); ++k ) {
      sentence.push_back ( ucam::util::toNumber<unsigned> ( aux[k] ) );
      if ( categories.find ( aux[k] ) == categories.end() ) {
        categories[aux[k]] = k + nnt + 1;
        cykdata_.cykgrid.Add ( k + nnt + 1, k, 0, sentence[k] );
      } else {
        cykdata_.cykgrid.Add ( categories[aux[k]], k, 0, sentence[k] );
      }
      vcat[k + nnt + 1] = aux[k];
    }
    USER_CHECK ( vcat[1] == "S",
                 "S must be the top guy in the non-terminal hierarchy" );
    USER_CHECK ( categories["S"] == 1,
                 "S must be the top guy in the non-terminal hierarchy (2)" );
    USER_CHECK ( aux.size() == sentence.size(),
                 "sentence not properly initialized?" );
    d_->stats->setTimeStart ("cyk");
    LDEBUG ( "Now start stage 1: Initialize Bottom row of the chart" );
    fillBottomChart(); ///stage 1
    LDEBUG ( "Start stage 2: fill bottom-up the chart" );
    fillChart();      ///stage 2
    d_->stats->setTimeEnd ("cyk");
    finalresult_ = cykdata_.cykgrid ( cykdata_.categories["S"], 0,
                                      sentence.size() - 1 ).size();
    if ( finalresult_ > 0 ) {
      LINFO ( "CYK success:" << finalresult_ << " parse trees" );
    } else {
      LINFO ( "CYK failed!" );
    }
    d.cykdata->storeRuleCounts ( d.stats->rulecounts );
    d.stats->numcats = d.cykdata->vcat.size();
    LINFO ( "Number of rules applied:" << d.stats->rulecounts.size() );
    cykdata_.success = finalresult_;
    return false;
  };

  ///Destructor
  ~CYKParserTask ( void ) {
    hmin_.clear();
    hmax_.clear();
  };

  /// returns cykdata structure
  const CYKdata *getcykdata() {
    return &cykdata_;
  };

  ///Returns  success (number of nodes in topmost cell) or failure (CYK_RETURNS_FAILURE=0)
  int getFinalResult() {
    return finalresult_;
  };

 private:

  /**
   * \brief       Recursive function that determines if a rule candidate is valid or not.
   * \param       unsigned x:    Position of the grid through the horizontal axis.
   * \param       unsigned ymax: maximum height (vertical axis) that can be examined for x.
   * \param       unsigned rule_idx: index of the rule we are examining.
   * \param       unsigned rule_pos: index of the element within the rule we have to examine (think of it as an Earley dot)
   * \param       vector<unsigned> coord: Coordinates required by that rule to backpoint. If empty, the rule is rejected.
   * \remarks     Recursive function that examines all the possible candidate rules beneath the triangle determined
   *              by a cell (x,ymax) and the string of words it spans. Every recursive call makes the triangle smaller, so ymax
   *              actually represents the diagonal of the triangle.
   *              The function will exit when a rule has been rejected/proved or the rightmost x of the original
   *              span has been reached.
   */

  void examineRule ( unsigned x, unsigned ymax, unsigned rule_idx,
                     unsigned rule_pos, cykparser_rulebpcoordinates_t coord ) {
    cykparser_ruledependencies_t& rd = cykdata_.rd;
    cykparser_rulebpcoordinates_t coord2;
    SentenceSpecificGrammarData& ssgd = *d_->ssgd;
    std::string cat =     ssgd.getRHSSource ( rule_idx, rule_pos );
    getFilteredNonTerminal ( cat );
    unsigned regla_tam = ssgd.getRHSSourceSize ( rule_idx );
    int x2;
    unsigned z = ( cykdata_.categories[cat] == 0 ) ? 0 : 1;
    for ( unsigned n = 0; n < ymax; n++ ) {
      if ( cykdata_.cykgrid ( cykdata_.categories[cat], x, n ).size() > 0 ) {
        coord2 = coord; //copy coord, we will need to reuse later.
        coord2.push_back ( cykdata_.categories[cat] );
        coord2.push_back ( x );
        coord2.push_back ( n );
        if ( rule_pos == regla_tam - 1 ) { //last element of the rule
          if ( n == ymax -
               1 ) { //We are in the diagonal, therefore span is complete -- full span verified.
            rd.push_back ( coord2 );
            //                    LDEBUG("span completely verified!, n="<<n<<",ymax="<<ymax);
          } else {
            //                    LDEBUG("span not verified, rule not valid, n="<<n<<",ymax="<<ymax);
          }
        } else {
          if ( ymax > 1 ) {
            x2 = n + x + 1; //next x.
            //This element verified, now look next one.
            examineRule ( x2, ymax - n - 1, rule_idx, rule_pos + 1, coord2 );
          }
        }
      }
      z = 0;
    }
    return;
  };

  /**
   * \remarks     Apply one single-element non-terminal rules (e.g. S->X).
   *              This function applies to cells c,x,y for all non-terminals or categories(c).
   *              Caution! Cyclic grammars (A->B, B->A) will create backpointers leading to infinite loops during traversal.
   * \param x     position x in the cyk grid.
   * \param y     position y in the cyk grid.
   */
  void insertMonoRHSRules ( unsigned x, unsigned y ) {
    cykparser_rulebpcoordinates_t coord;
    grammar_categories_t& categories = cykdata_.categories;
    grammar_inversecategories_t& vcat = cykdata_.vcat;
    unsigned& nnt = cykdata_.nnt; //number of non-terminals
    SentenceSpecificGrammarData& ssgd = *d_->ssgd;
    ssgrammar_listofrules_t rules;
    for ( unsigned cc = nnt; cc > 1;
          --cc ) { // cc>1 as for 1 there will be no rule with cc lower.
      if ( cykdata_.cykgrid ( cc, x, y ).size() > 0 ) {
        LDEBUG ( vcat[cc] << "," << x << "," << y << ":" <<
                 "Searching for rules with only one element" );
        if ( ssgd.rulesWithRhsSpan1[x].find ( vcat[cc] ) !=
             ssgd.rulesWithRhsSpan1[x].end() )
          rules = ssgd.rulesWithRhsSpan1[x][vcat[cc]];
        else rules.clear();
        LDEBUG ( "Found: " << rules.size() );
        for ( unsigned jpts = 0; jpts < rules.size(); ++jpts ) {
          unsigned& idx = rules[jpts];
          unsigned head = categories[ssgd.getLHS ( idx )];
          //          if ( head <= cykdata_.numberTopNonTerminals && x > 0 ) continue;
          //if not listed as exception, then discard
          if ( y >= hrmh_ &&
               cykdata_.nt_exceptions_maxspan.find ( ssgd.getLHS ( idx ) ) ==
               cykdata_.nt_exceptions_maxspan.end() ) continue;
          if ( head >= cc ) {
            LWARN ( ssgd.getLHS ( idx ) << "," << x << "," << y << ":" <<
                    "Potentially nasty looping rule (filtering): " << ssgd.getRule ( idx ) );
            continue;
          }
          LDEBUG ( ssgd.getLHS ( idx ) << "," << x << "," << y << ":" << "Accepted: " <<
                   ssgd.getRule ( idx )  << ", original rule_id=" << ssgd.getIdx (
                     idx ) << ",vs" << idx );
          cykdata_.cykgrid.Add ( head, x, y, idx );
          coord.clear();
          coord.push_back ( cc );
          coord.push_back ( x );
          coord.push_back ( y );
          cykdata_.bp.Add ( head, x, y, coord );
        }
      }
    }
  };

  /**
   * \brief     stage 1: initialize the chart (i.e. putting words and applying one-element rules to y=0 row).
   */

  void fillBottomChart() {
    grammar_categories_t& categories = cykdata_.categories;
    cykparser_sentence_t& sentence = cykdata_.sentence;
    //    const GrammarData &gd = *d_->grammar;
    SentenceSpecificGrammarData& ssgd = *d_->ssgd;
    ssgrammar_listofrules_t rules;
    for ( unsigned x = 0; x < sentence.size(); ++x ) {
      std::string cat = ucam::util::toString ( sentence[x] );
      if ( ssgd.rulesWithRhsSpan1[x].find ( cat ) !=
           ssgd.rulesWithRhsSpan1[x].end() ) {
        rules = ssgd.rulesWithRhsSpan1[x][cat]; //In this case it should be identical...
      } else {
        LDEBUG ( "No rules to process" );
        rules.clear();
      }
      for ( unsigned p = 0; p < rules.size(); ++p ) {
        unsigned& idx = rules[p];
        LDEBUG ( ssgd.getLHS ( idx ) << "," << x << ",0:" << "Accepted: " <<
                 ssgd.getRule ( idx ) << ", original rule_id=" << ssgd.getIdx (
                   idx ) << ",vs" << idx );
        cykdata_.cykgrid.Add ( categories[ssgd.getLHS ( idx )], x, 0, idx );
        cykparser_rulebpcoordinates_t cdi;
        cdi.push_back ( categories[cat] );
        cdi.push_back ( x );
        cdi.push_back ( 0 ); //pointing to the word itself.
        cykdata_.bp.Add ( categories[ssgd.getLHS ( idx )], x, 0, cdi );
      }
      insertMonoRHSRules ( x, 0 );
    }
  };

  /**
   * \brief     stage 2: fill the chart,rows y>=1.
   * \remarks   Traverses the cykgrid for all cc, x and y>=1 in a bottom-up fashion.
   * At a given x,y, it inspects all lower level cells cc,x1,y1 (x1=x,0<y1<y) to find
   * a candidate rule applicable to LHS(rule),x,y.
   * If any rule exists and is considered to be valid under various filtering parameters
   * hmin,hmax, target vocabulary restriction (with a potential for large etc of filtering options),
   * the recursive examineRules is called to determine which ones actually apply (i.e. fully cover
   * the span defined by x,y). In that case, the cykgrid and the backpointers are updated.
   * After inserting rules with only one non-terminal in the RHS, we can move to the next cell.
   * Each rule backpoints to as many lower level cells (containing many rule indices and so on)
   * as non-terminals in RHS-source(rule). The search space described in this way is complete.
   * Note that the cyk grid only stores the rule index, no weights are considered, as
   * these are not needed until the translation step.
   */

  void fillChart() {
    grammar_categories_t& categories = cykdata_.categories;
    grammar_inversecategories_t&  vcat = cykdata_.vcat;
    cykparser_sentence_t&  sentence = cykdata_.sentence;
    unsigned& nnt = cykdata_.nnt;
    unordered_map<std::string, cykparser_ruledependencies_t  > minicache;
    cykparser_ruledependencies_t&  rd = cykdata_.rd;
    cykparser_rulebpcoordinates_t coord;
    const GrammarData& gd = *d_->grammar;
    SentenceSpecificGrammarData& ssgd = *d_->ssgd;
    ssgrammar_listofrules_t rules;
    for ( unsigned y = 1; y < sentence.size(); ++y ) {
      for ( unsigned x = 0; x < sentence.size() - y; ++x ) {
        for ( unsigned n = 0; n < y; n++ ) {
          //just an alias.
          unsigned& x1 = x;
          unsigned y1 = n;
          unsigned x2 = y1 + x1 + 1;
          for ( unsigned cc = 1; cc <= nnt + sentence.size();
                ++cc ) { //Only in this case we have to make sure it exists.
            std::string cat = vcat[cc];
            // prevent non terminals other than the top nonterminals (e.g. S) to have a span greater or equal to hrmaxheight
            if ( cykdata_.cykgrid ( cc, x1, y1 ).size() == 0 ) continue;
            //Keep caching at the same position for identical rules with different translations
            minicache.clear();
            rules = ssgd.rulesWithRhsSpan2OrMore[x][cat];
            LDEBUG ( vcat[cc] << "," << x << "," << y << ":" << " Obtained " << toString (
                       rules.size() ) << " rules." );
            for ( unsigned i = 0; i < rules.size(); i++ ) {
              unsigned idx = rules[i];
              LDEBUG ( vcat[cc] << "," << x << "," << y << ":" << "Testing rule idx=" << idx
                       << ",value=" << ssgd.getRule ( idx )  << "; Iteration=" << i );
              if ( ssgd.getRHSSourceSize ( idx ) > y + 1 ) {
                LDEBUG ( vcat[cc] << "," << x << "," << y << ":" <<
                         "Rejected (rule minimum span too big): " << ssgd.getRule ( idx ) );
                continue;
              }
              if (y >= hrmh_ ) {
                if (cykdata_.nt_exceptions_maxspan.find (ssgd.getLHS ( idx ) ) ==
                    cykdata_.nt_exceptions_maxspan.end() ) {
                  LDEBUG ( vcat[cc] << "," << x << "," << y << ":" <<
                           "Rejected (Default Maximum span reached hrmaxheight): " << ssgd.getRule (
                             idx ) );
                  LDEBUG ( "Note: hrmaxheight=" << hrmh_ <<
                           " has been reached, only nonterminals listed as exceptions (--cykparser.ntexceptionsmaxspan are allowed!" );
                  continue;
                }
              }
              if ( hmax_.find ( ssgd.getLHS ( idx ) ) != hmax_.end() )
                if ( y > hmax_[ssgd.getLHS ( idx )] ) {
                  LDEBUG ( vcat[cc] << "," << x << "," << y << ":" << "Rejected (hmax): " <<
                           ssgd.getRule ( idx ) );
                  continue;
                }
              if ( hmin_.find ( ssgd.getLHS ( idx ) ) != hmin_.end() )
                if ( y < hmin_[ssgd.getLHS ( idx )] ) {
                  LDEBUG ( vcat[cc] << "," << x << "," << y << ":" << "Rejected (hmin): " <<
                           ssgd.getRule ( idx ) );
                  continue;
                }
              std::string rhs = ssgd.getRHSSource ( idx );
              rd.clear();
              unordered_map<std::string, cykparser_ruledependencies_t > ::iterator itx =
                minicache.find ( rhs );
              if ( itx != minicache.end() ) {
                rd = itx->second; /// memoization: we don't need to call examinerules twice for the same source-side rule.
              } else {
                coord.clear();
                coord.push_back ( cc );
                coord.push_back ( x1 );
                coord.push_back ( y1 );
                examineRule ( x2 - 0, y - ( x2 - x1 - 1 ), idx, 1, coord );
                minicache[rhs] = rd; ///Memoization: keep track of this RHS
              }
              if ( rd.size() == 0 ) {
                LDEBUG ( vcat[cc] << "," << x << "," << y << "(/" << n << "):" << "Rejected " <<
                         ssgd.getRule ( idx ) << ", for: " << ssgd.getLHS ( idx ) << "," << x << "," <<
                         y );
                continue;
              }
              LDEBUG ( vcat[cc] << "," << x << "," << y << "(/" << n << "):" << "Accepted: "
                       << ssgd.getRule ( idx )  << ", for: " << ssgd.getLHS ( idx ) << "," << x << ","
                       << y << ", original rule_id=" << ssgd.getIdx ( idx ) << ",vs" << idx );
              for ( unsigned kpt = 0; kpt < rd.size(); kpt++ ) {
                cykdata_.cykgrid.Add ( categories[ssgd.getLHS ( idx )], x, y , idx );
                cykdata_.bp.Add ( categories[ssgd.getLHS ( idx )], x, y, rd[kpt] );
              }
            }
          }
        }
        insertMonoRHSRules ( x, y );
      }
    }
  };

  ZDISALLOW_COPY_AND_ASSIGN ( CYKParserTask );

};

}
} // end namespaces

#endif
