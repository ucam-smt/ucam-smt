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

#ifndef TASK_HIFST_HPP
#define TASK_HIFST_HPP

/**
 * \file
 * \brief Contains structures and classes for hifst task (target lattice building)
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include "task.hifst.replacefstbyarc.hpp"
#include "task.hifst.expandednumstates.hpp"
#include "task.hifst.localpruningconditions.hpp"
#include "task.hifst.rtn.hpp"
#include "task.hifst.optimize.hpp"
#include "task.hifst.makeweights.hpp"
#include "task.applylm.kenlmtype.hpp"
namespace ucam {
namespace hifst {

/**
 * \brief Core of Hifst. Implements the lattice-building procedure for a cyk-parsed sentence
 */

template <class Data ,
          class Arc = fst::LexStdArc ,
          class OptimizeT = OptimizeMachine<Arc> ,
          class CYKdataT = CYKdata ,
          class MultiUnionT = fst::MultiUnionRational<Arc> ,
          //          class MultiUnionT = fst::MultiUnionReplace<Arc> ,
          class ExpandedNumStatesRTNT = ExpandedNumStatesRTN<Arc> ,
          class ReplaceFstByArcT = ManualReplaceFstByArc<Arc> ,
          class RTNT = RTN<Arc>
          >
class HiFSTTask: public ucam::util::TaskInterface<Data> {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

  //Private variables are shown here. Private methods go after public methods
 private:

  ///\todo these three need some careful thought
  //  unsigned sc_;
  unsigned piscount_;
  std::set<Label> hieroindexexistence_;

  OptimizeT optimize;

  ////////////////////////////////////////////////////////

  /// Use hipdt mode or not
  bool hipdtmode_;

  ///Use ReplaceUtil or not to optimize RTNs
  bool rtnopt_;

  /// Pointer to the general data structure.
  Data *d_;

  ///Non-terminals for which its cell lattices will be replaced by a trivial arc, o-o .
  ReplaceFstByArcT *rfba_;
  unordered_set<std::string> replacefstbyarc_;
  unordered_set<std::string> replacefstbyarcexceptions_;
  unsigned replacefstbynumstates_;

  /// Pointer to the cyk data structure.
  CYKdataT *cykdata_;

  /// Aligner mode, i.e. derivations encoded in the left-side of the machines.
  bool aligner_;

  ///Enable epsilon removal+determinization+minimization right after finishing delayed lattice construction.
  ///Disabled
  ///  bool finalredm_;

  ///Enable epsilon removal+determinization+minimization after pruning at each cell.
  ///\todo cellredm_ not implemented!
  ///  bool cellredm_;

  ///Pdt parentheses.
  std::vector<pair<Label, Label> > pdtparens_;

  ///Delayed fsts stored in a cyk-grid-like structure.

  RTNT *rtn_;

  ExpandedNumStatesRTNT *rtnnumstates_;

  ///Vector of label-fst pairs, for replacement.
  std::vector< std::pair< Label, const fst::Fst<Arc> * > > pairlabelfsts_;

  ///Result stored here!
  fst::VectorFst<Arc> cykfstresult_;

  ///To local prune or not to local prune
  bool localprune_;
  /// Number of local language models for hifst
  unsigned numlocallm_;

  /// 4-tuples of conditions
  std::vector<std::string> lpctuples_;

  /// checks whether it qualifies or not for local pruning
  LocalPruningConditions lpc_;

  /// Defines a weight in the appropriate semiring (Lex, Std , or TupleArc)
  MakeWeightHifst<Arc> mw_;

  /// Likelihood weight
  float pruneweight_;

  //where to store rtn files
  ucam::util::IntegerPatternAddress rtnfiles_;

  //Input/output keys
  const std::string lmkey_;
  const std::string locallmkey_;
  const std::string outputkey_;
  const std::string fullreferencelatticekey_;

  //To avoid multiple warnings on logs per sentence
  bool warned_;

  // If in translation after cell pruning the number of states of a lattice is bigger than numstatethresholdafterpruning_,
  // then the lattice will not be determinized/minimized.
  unsigned numstatesthreshold_;

  //If false, no determinization/minimization will be applied anywhere to any of the components of the RTN, expanded or not.
  bool optimize_;
  const ucam::util::RegistryPO& rg_;
  //  const int localLmPos_;
 public:

  ///Constructor with registry object and several keys to access data object and registry
  HiFSTTask ( const ucam::util::RegistryPO& rg ,
              const std::string& outputkey = HifstConstants::kHifstLatticeStore,
              const std::string& locallmkey = HifstConstants::kHifstLocalpruneLmLoad,
              const std::string& fullreferencelatticekey =
                HifstConstants::kReferencefilterNosubstringStore ,
              const std::string& lmkey = HifstConstants::kLmLoad
            ) :
      optimize_ (rg.getBool (HifstConstants::kHifstOptimizecells) ),
      numlocallm_ (rg.getVectorString (locallmkey).size() ),
      warned_ (false),
      rtnfiles_ (rg.get<std::string> (HifstConstants::kHifstWritertn) ),
      fullreferencelatticekey_ ( fullreferencelatticekey ),
      lmkey_ ( lmkey ),
      locallmkey_ ( locallmkey ),
      outputkey_ ( outputkey ),
      piscount_ ( 0 ),
      aligner_ ( rg.getBool ( HifstConstants::kHifstAlilatsmode ) ),
      //    cellredm_ ( rg.getBool ( "hifst.cellredm" ) ),
      //    finalredm_ ( rg.getBool ( "hifst.finalredm" ) ),
      hipdtmode_ (rg.getBool (HifstConstants::kHifstUsepdt) ),
      rtnopt_ (rg.getBool (HifstConstants::kHifstRtnopt) ),
      replacefstbyarc_ ( rg.getSetString (
                             HifstConstants::kHifstReplacefstbyarcNonterminals ) ),
      replacefstbyarcexceptions_ ( rg.getSetString (
                                       HifstConstants::kHifstReplacefstbyarcExceptions ) ),
      replacefstbynumstates_ ( rg.get<unsigned>
                               ( HifstConstants::kHifstReplacefstbyarcNumstates ) ),
      localprune_ ( rg.getBool ( HifstConstants::kHifstLocalpruneEnable ) ),
      pruneweight_ ( rg.get<float> ( HifstConstants::kHifstPrune ) ),
      numstatesthreshold_ ( rg.get<unsigned>
                            ( HifstConstants::kHifstLocalpruneNumstates ) ),
      lpctuples_ ( rg.getVectorString (
                       HifstConstants::kHifstLocalpruneConditions ) ),
      mw_(rg),
      rg_(rg)
      //      localLmPos_(rg.getVectorString(HifstConstants::kLmFeatureweights).size() + 1 + 1)
  {

    LINFO ("Number of local language models=" << numlocallm_);
    LINFO ("aligner mode=" << aligner_);
    LINFO ("localprune mode=" << localprune_);
    LINFO("reference filtering with: " << rg_.get<std::string> (HifstConstants::kReferencefilterLoad));
    USER_CHECK ( ! ( lpc_.size() % 4 ),
                 "local pruning conditions are defined by tuples of 4 elements: category,x,y,Number-of-states. Category is a string and x,y are int. Number of states is unsigned" );
    USER_CHECK ( (localprune_ && numlocallm_ )
                 || ( localprune_ && !numlocallm_  && rg_.get<std::string> (HifstConstants::kReferencefilterLoad) != ""  )
                 || (!localprune_) ,
                 "If you want to do cell pruning in translation, you should  normally use a language model for local pruning. Check --hifst.localprune.lm.load and --hifst.localprune.enable.\n");
    optimize.setAlignMode (aligner_);



    if (hipdtmode_) {
      LINFO ("Hipdt mode enabled!");
    }
    if (!rtnopt_) {
      LINFO ("RTN openfst optimizations will not be applied");
    }
    LDEBUG ( "Hifst constructor done!" );
  };

  /**
   * \brief Runs the lattice building procedure.
   * \param d          Contains the data structure with all the necessary elements (i.e. cykdata) and in which will be store a pointer to the
   * output lattice.
   */
  bool run ( Data& d ) {
    cykfstresult_.DeleteStates();
    this->d_ = &d;
    hieroindexexistence_.clear();
    LINFO ( "Running HiFST" );
    //Reset one-time warnings for inexistent language models.
    warned_ = false;
    pdtparens_.clear();
    cykdata_ = d.cykdata;
    if ( !USER_CHECK ( cykdata_, "cyk parse has not been executed previously?" ) ) {
      resetExternalData (d);
      return true;
    }
    if ( d.cykdata->success == CYK_RETURN_FAILURE ) {
      ///Keep calm, return empty lattice and carry on
      fst::VectorFst<Arc> aux;
      d.fsts[outputkey_] = &cykfstresult_;
      d.vcat = cykdata_->vcat;
      resetExternalData (d);
      return false;
    }
    ///If not yet, initialize now functor with local conditions.
    initLocalConditions();
    rtn_ = new RTNT;
    if ( localprune_ )
      rtnnumstates_ = new ExpandedNumStatesRTNT;
    rfba_ = new ReplaceFstByArcT ( cykdata_->vcat, replacefstbyarc_,
                                   replacefstbyarcexceptions_, aligner_, replacefstbynumstates_ );
    piscount_ = 0; //reset pruning-in-search count to 0
    LINFO ( "Second Pass: FST-building!" );
    d.stats->setTimeStart ( "lattice-construction" );
    //Owned by rtn_;
    fst::Fst<Arc> *sfst = buildRTN ( cykdata_->categories["S"], 0,
                                     cykdata_->sentence.size() - 1 );
    d.stats->setTimeEnd ( "lattice-construction" );
    cykfstresult_ = (*sfst);
    LINFO ( "Final - RTN head optimizations !" );
    optimize ( &cykfstresult_ ,
               std::numeric_limits<unsigned>::max() ,
               !hipdtmode_  && optimize_
             );
    FORCELINFO ("Stats for Sentence " << d.sidx <<
                ": local pruning, number of times=" << piscount_);
    d.stats->lpcount = piscount_; //store local pruning counts in stats
    LINFO ("RTN expansion starts now!");
    //Expand...
    {
      ///Define hieroindex
      Label hieroindex = APBASETAG + 1 * APCCTAG + 0 * APXTAG +
                         ( cykdata_->sentence.size() - 1 ) * APYTAG;
      if ( hieroindexexistence_.find ( hieroindex ) == hieroindexexistence_.end() )
        pairlabelfsts_.push_back ( pair< Label, const fst::Fst<Arc> * > ( hieroindex,
                                   &cykfstresult_ ) );
      ///Optimizations over the rtn -- they are generally worth doing...
      fst::ReplaceUtil<Arc> replace_util (pairlabelfsts_, hieroindex,
                                          !aligner_); //has ownership of modified rtn fsts
      if (rtnopt_) {
        LINFO ("rtn optimizations...");
        d_->stats->setTimeStart ("replace-opts");
        replace_util.ReplaceTrivial();
        replace_util.ReplaceUnique();
        replace_util.Connect();
        pairlabelfsts_.clear();
        replace_util.GetFstPairs (&pairlabelfsts_);
        d_->stats->setTimeEnd ("replace-opts");
      }
      //After optimizations, we can write RTN if required by user
      writeRTN();
      boost::scoped_ptr< fst::VectorFst<Arc> > efst (new fst::VectorFst<Arc>);
      if (!hipdtmode_ ) {
        LINFO ("Final Replace (RTN->FSA), main index=" << hieroindex);
        d_->stats->setTimeStart ("replace-rtn-final");
        Replace (pairlabelfsts_, &*efst, hieroindex, !aligner_);
        d_->stats->setTimeEnd ("replace-rtn-final");
      } else {
        LINFO ("Final Replace (RTN->PDA)");
        d_->stats->setTimeStart ("replace-pdt-final");
        Replace (pairlabelfsts_, &*efst, &pdtparens_, hieroindex);
        d_->stats->setTimeEnd ("replace-pdt-final");
        LINFO ("Number of pdtparens=" << pdtparens_.size() );
      }

      // Currently no need to call this applyFilters: it will do the same
      // and it is more efficient to compose with the normal lattice
      // rather than the substringed lattice.
      // LINFO ("Removing Epsilons...");
      // fst::RmEpsilon<Arc> ( &*efst );
      //  LINFO ("Done! NS=" << efst->NumStates() );
      // applyFilters ( &*efst );
      //Compose with full reference lattice to ensure that final lattice is correct.
      if ( d.fsts.find ( fullreferencelatticekey_ ) != d.fsts.end() ) {
        if ( static_cast< fst::VectorFst<Arc> * >
             (d.fsts[fullreferencelatticekey_])->NumStates() > 0 ) {
          LINFO ( "Composing with full reference lattice, NS=" <<
                  static_cast< fst::VectorFst<Arc> * >
                  (d.fsts[fullreferencelatticekey_])->NumStates() );
          fst::Compose<Arc> ( *efst,
                              * ( static_cast<fst::VectorFst<Arc> * > (d.fsts[fullreferencelatticekey_]) ),
                              &*efst );
          LINFO ( "After composition: NS=" << efst->NumStates() );
        } else {
          LINFO ( "No composition with full ref lattice" );
        };
      } else {
        LINFO ( "No composition with full ref lattice" );
      };
      //Apply language model
      fst::VectorFst<Arc> *res = NULL;
      if (efst->NumStates() )
        res = applyLanguageModel ( *efst  );
      else {
        LWARN ("Empty lattice -- skipping LM application");
      }
      if ( res != NULL ) {
        boost::shared_ptr<fst::VectorFst<Arc> >latlm ( res );
        if ( latlm.get() == efst.get() ) {
          LWARN ( "Yikes! Unexpected situation! Will it crash? (muhahaha) " );
        }
        //Todo: union with shortest path...
        if ( pruneweight_ < std::numeric_limits<float>::max() ) {
          if (!hipdtmode_ || pdtparens_.empty() ) {
            LINFO ("Pruning, weight=" << pruneweight_);
            fst::Prune<Arc> (*latlm, &cykfstresult_, mw_ ( pruneweight_ ) );
          } else {
            LINFO ("Expanding, weight=" << pruneweight_);
            fst::ExpandOptions<Arc> eopts (true, false, mw_ ( pruneweight_ ) );
            Expand ( *latlm, pdtparens_, &cykfstresult_, eopts);
            pdtparens_.clear();
          }
        } else {
          LINFO ("Copying through full lattice with lm scores");
          cykfstresult_ = *latlm;
        }
      } else {
        LINFO ("Copying through full lattice (no lm)");
        cykfstresult_ = *efst;
      }
      if ( hieroindexexistence_.find ( hieroindex ) == hieroindexexistence_.end() )
        pairlabelfsts_.pop_back();
    }
    pairlabelfsts_.clear();
    LINFO ( "Reps" );
    fst::RmEpsilon ( &cykfstresult_ );
    LINFO ( "NS=" << cykfstresult_.NumStates() );
    //This should delete all pertinent fsas...
    LINFO ( "deleting data stuff..." );
    delete rtn_;
    if ( localprune_ )
      delete rtnnumstates_;
    delete rfba_;
    d.vcat = cykdata_->vcat;
    resetExternalData (d);
    d.fsts[outputkey_] = &cykfstresult_;
    if (hipdtmode_ && pdtparens_.size() )
      d.fsts[outputkey_ + ".parens" ] = &pdtparens_;
    LINFO ( "done..." );
    FORCELINFO ( "End Sentence ******************************************************" );
    d.stats->setTimeEnd ( "sent-dec" );
    d.stats->message += "[" + ucam::util::getTimestamp() + "] End Sentence\n";
    return false;
  };

 private:

  ///Clear elements in the data object
  inline void resetExternalData (Data& d) {
    cykdata_->freeMemory();
    d.tvcb.clear();
    d.filters.clear();
  }

  ///Write RTN componentes to disk
  void writeRTN() {
    //Dump to disk all the FSAs for this  RTN.
    if (rtnfiles_() != "") {
      std::string filenamepattern = rtnfiles_ (d_->sidx);
      FORCELINFO ("Writing rtn files..." << filenamepattern);
      for (unsigned k = 0; k < pairlabelfsts_.size(); ++k) {
        std::string filename = filenamepattern;
        ucam::util::find_and_replace (filename, "%%rtn_label%%"
                                      , ucam::util::toString<Label> (pairlabelfsts_[k].first) );
        fst::FstWrite (static_cast< fst::VectorFst<Arc> const& > (*
                       (pairlabelfsts_[k].second) ), filename);
      }
    }
  };

  /**
   * \remarks     Reorder fst dependencies to match the order of non-terminals of the rule
   * \param       unsigned rule_idx: index of the rule in RuleManager.
   * \param       vector < Fst < Arc > * > &fsts: vector of pointers to fsts in lower cells.
   * \retval      void.
   * \return      parameter fsts has the expected order of lower lattices corresponding to the order of non-terminals of the rule
   */
  inline void mapfsts ( unsigned int rule_idx,
                        std::vector < fst::Fst < Arc > * >& fsts ) {
    unordered_map<unsigned int, unsigned int > mappings;
    d_->ssgd->getMappings ( rule_idx, &mappings );
    USER_CHECK ( fsts.size() == mappings.size(),
                 "Mismatch between mappings and lower-level fsts" );
    LDEBUG ( "mappings size=" << mappings.size() );
    std::vector<fst::Fst<Arc>* > newfsts ( fsts.size(), NULL );
    for ( unsigned int k = 0; k < fsts.size(); k++ )
      newfsts[mappings[k]] = fsts[k];
    fsts = newfsts;
  };

  /**
   * \remarks     Recursive algorithm that traverses the backpointers from the CYK grid.
   *              It builds local fsts for each rule. If rule is hierarchical, it calls this function
   *              in order to obtain first the fst associated to the cell.
   *              Fsts for all the rules are unioned.
   *              Finally, if conditions apply, there is a pruning in search and replacement of fsts by arcs
   * \param       unsigned cc: category axis of the grid.
   * \param       unsigned x: horizontal axis of the grid
   * \param       unsigned y: vertical axis of the grid
   * \retval      Pointer to the Fst generated for this cell.
   */
  fst::Fst<Arc>* buildRTN ( unsigned int cc, unsigned int x, unsigned int y ) {
    fst::Fst<Arc> *ptr = ( *rtn_ ) ( cc, x, y );
    if ( ptr != NULL ) return ptr;
#ifdef PRINTDEBUG
    std::ostringstream o;
    o << cc << "." << x << "." << y;
#endif
    unsigned& nnt = cykdata_->nnt;
    grammar_inversecategories_t& vcat = cykdata_->vcat;
    std::stringstream ostr;
    ostr << vcat[cc] << "." << x << "." << y;
    std::string filename;
    ostr >> filename;
    SentenceSpecificGrammarData& g = *d_->ssgd;
    MultiUnionT mur;
    Label hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    LDEBUG ( "bp> " << cc << "," << x << "," << y << ":" <<
             ( unsigned ) cykdata_->bp ( cc, x, y ).size() );
    for ( unsigned i = 0; i < cykdata_->bp ( cc, x, y ).size(); i++ ) {
      unsigned idx = cykdata_->cykgrid ( cc, x, y, i );
      std::vector<fst::Fst<Arc>*> requiredfsts;
      if ( g.isPhrase ( idx ) ) {
        mur.Add ( addRule ( idx, requiredfsts ) ) ;
        LDEBUG ( "AT " << cc << "," << x << "," << y <<
                 ":adding phrase-based rule index " << idx );
        continue;
      }
      const cykparser_ruledependencies_t& mybp = cykdata_->bp ( cc, x, y );
      for ( unsigned j = 0; j < mybp[i].size(); j += 3 ) {
        if ( mybp[i][j] > nnt ) {
          continue;
        }
        requiredfsts.push_back ( buildRTN ( mybp[i][j], mybp[i][j + 1],
                                            mybp[i][j + 2] ) );
        LDEBUG ( "back to bp> " << cc << "," << x << "," << y << ":" <<
                 ( unsigned ) cykdata_->bp ( cc, x, y ).size() );
      }
      mapfsts ( idx, requiredfsts );
      LDEBUG ( "AT " << cc << "," << x << "," << y << ": adding hiero rule index " <<
               idx );
      mur.Add ( addRule ( idx, requiredfsts ) );
    }
    boost::shared_ptr< fst::VectorFst<Arc> >  mdfst ( mur() );
    LDBG_EXECUTE ( mdfst->Write ( "fsts/" + o.str() + ".fst" ) );
    //Optimize
    optimize ( &*mdfst ,
               std::numeric_limits<unsigned>::max(),
               optimize_ );
    LDEBUG ( "AT " << cc << "," << x << "," << y << ": FST built!" );
    LDBG_EXECUTE ( mdfst->Write ( "fsts/" + o.str() + "redm.fst" ) );
    d_->stats->numstates[ cc * 1000000 + y * 1000 + x  ] =
      mdfst->NumStates();  //Just store the number of states of the not-expanded FSA.
    //Calculate expanded number of states of the partial rtn.
    if ( localprune_ )
      rtnnumstates_->update ( cc, x, y, &*mdfst );
    boost::scoped_ptr< fst::VectorFst<Arc> > pruned ( localPruning ( *mdfst, cc, x,
        y ) );
    //We now might have a pruned lattice!
    if ( pruned.get() != NULL ) {
      optimize (&*pruned , numstatesthreshold_ , !hipdtmode_  && optimize_ );
      *mdfst = *pruned;
      //Only if we prune, we add to stats total number of states of full and pruned lattice
      d_->stats->numstates[ cc * 1000000 + y * 1000 + x  ] = ( *rtnnumstates_ ) ( cc,
          x,
          y );
      rtnnumstates_->update ( cc, x, y,
                              &*mdfst );       //Update rtnnumstates again...
      d_->stats->numprunedstates[ cc * 1000000 + y * 1000 + x ] = ( *rtnnumstates_ ) (
            cc, x, y );
    } else {
      LDEBUG ( "AT " << cc << "," << x << "," << y << ":No pruning" );
    }
    boost::shared_ptr< fst::VectorFst<Arc> > outfst ( ( *rfba_ ) ( *mdfst,
        hieroindex ) );
    if ( outfst.get() != NULL ) {
      LDEBUG ( "AT " << cc << "," << x << "," << y << ": replacefstbyarcfor cat= " <<
               vcat[cc] << ",NS=" << mdfst->NumStates() );
      rtn_->Add ( cc, x, y, outfst , mdfst );
      hieroindexexistence_.insert ( hieroindex );
      pairlabelfsts_.push_back ( pair< Label, const fst::Fst<Arc> * > ( hieroindex,
                                 &*mdfst ) );
    } else {
      rtn_->Add ( cc, x, y, mdfst , outfst );
      LDEBUG ( "AT: " << cc << "," << x << "," << y << ":" <<
               "Delaying not applied. Stored, NS=" << ( unsigned ) mdfst->NumStates() );
    }
    return ( *rtn_ ) ( cc, x, y );
  };

  /**
   * \brief creates a new fst based on a rule and lower-level cell lattices.
   * \param rule_idx: Rule (sorted) index
   * \param &lowerfsts: Lower level cell lattices required to build a rule.
   * \returns pointer to a newly created fst.
   */

  fst::VectorFst<Arc> *addRule ( unsigned rule_idx,
                                 std::vector<fst::Fst<Arc>*>& lowerfsts ) {
    SentenceSpecificGrammarData& gd = *d_->ssgd;
    std::vector<std::string> translation = gd.getRHSSplitTranslation ( rule_idx );
    if ( !translation.size() ) {
      LERROR ( gd.getRule ( rule_idx ) );
      translation.push_back ( 0 );
    }
    for (unsigned k = 0; k < translation.size(); ++k) {
      if ( translation[k] == "<s>" ) {
        translation[k] = "1";
      } else if ( translation[k] == "</s>" ) translation[k] = "2";
      else if ( translation[k] == "<dr>" ) {
        std::stringstream dr;
        dr << DR;
        translation[k] = dr.str();
        LDEBUG ( "Deletion rule: " << gd.getRule ( rule_idx ) << "," <<
                 translation[k] );
      } else if ( translation[k] == "<oov>" ) {
        std::stringstream oov;
        oov << OOV;
        translation[k] = oov.str();
        LDEBUG ( "oov rule: " << gd.getRule ( rule_idx ) << "," << translation[k] );
      } else if ( translation[k] == "<sep>" ) {
        std::stringstream sep;
        sep << SEP;
        translation[k] = sep.str();
        LDEBUG ( "separator rule: " << gd.getRule ( rule_idx ) << "," <<
                 translation[k] );
      }
    }
    LDEBUG ( "Starting to build!" );
    fst::VectorFst<Arc> *rulefst = new fst::VectorFst<Arc>;
    rulefst->AddState();
    rulefst->SetStart ( 0 );
    rulefst->AddState();
    Label iw2 = gd.getIdx ( rule_idx ) + 1;
    Label iw;
    if ( !aligner_ ) iw = 0;
    else iw = iw2;
    LINFO ("Building FST for rule " << gd.getRule ( rule_idx ) );
    unsigned kmax = translation.size();
    unsigned nonterminal = 0;
    std::vector< pair< Label, const fst::Fst<Arc> * > > pairlabelfsts;
    for ( unsigned k = 0; k < kmax; k++ ) {
      //if non-terminal... just place special arc and expand later...
      Label ow;
      if ( !isTerminal ( translation[k] ) ) {
        ow = APRULETAG + nonterminal;
        USER_CHECK ( lowerfsts.size() > nonterminal,
                     "Missing fsts to build the rule..." );
        pairlabelfsts.push_back ( pair< Label, const fst::Fst<Arc> * > ( ow,
                                  lowerfsts[nonterminal++] ) );
      } else {
        std::istringstream buffer ( translation[k] );
        buffer >> ow;
      }
      rulefst->AddState();
      Label iw;
      if ( !aligner_ ) iw = ow;
      else iw = NORULE;
      rulefst->AddArc ( k, Arc ( iw, ow, Weight::One(), k + 1 ) );
    }
    float w = gd.getWeight ( rule_idx );
    Weight weight = mw_ ( w , iw2 );
    rulefst->AddArc ( kmax, Arc ( iw, 0, weight, kmax + 1 ) );
    rulefst->SetFinal ( kmax + 1, Weight::One() );
    fst::VectorFst<Arc>* auxi;
    if ( nonterminal > 0 ) {
      pairlabelfsts.push_back ( pair< Label, const fst::Fst<Arc> * >
                                ( APRULETAG + nonterminal, rulefst ) );
      fst::VectorFst<Arc> *aux = new fst::VectorFst<Arc>;
      Replace (pairlabelfsts, aux, APRULETAG + nonterminal, !aligner_);
      delete rulefst;
      rulefst = aux;
    }
    fst::RmEpsilon<Arc> ( rulefst );
    return rulefst;
  }

  //Note that local conditions can be sentence-specific
  void initLocalConditions() {
    if ( !localprune_ ) return;
    if ( !lpctuples_.size() ) return;
    lpc_.clear();
    LINFO ( "Set up conditions for local cell pruning" );
    for ( unsigned k = 0; k < lpctuples_.size(); k += 4 ) {
      int y = ucam::util::toNumber<int> ( lpctuples_[k + 1] );
      if ( y < 0 ) y = cykdata_->getNumberWordsSentence() + y + 1;
      LINFO ( "cell pruning conditions (cat,span,numstates,weight): "
              << cykdata_->categories[lpctuples_[k]]
              << "," << y << ","
              << ucam::util::toNumber<unsigned> ( lpctuples_[k + 2] ) << ","
              << ucam::util::toNumber<unsigned> ( lpctuples_[k + 3] ) );
      conditions c ( cykdata_->categories[lpctuples_[k]]
                     , y
                     , ucam::util::toNumber<unsigned> ( lpctuples_[k + 2] )
                     , ucam::util::toNumber<unsigned> ( lpctuples_[k + 3] ) );
      lpc_.add ( c );
    }
    LINFO ( "We have: " << lpc_.size() << " conditions" );
  };

  /**
   * \brief Applies any fst filters created in previous tasks and stored d.filters.
   * These are right-side filters: expected to work only with the right side of the translation lattice (e.g. left side can be derivations).
   * \param *fst    : Input/output lattice
   */

  inline void applyFilters ( fst::VectorFst<Arc> *fst ) {
    fst::ArcSort<Arc> ( fst, fst::OLabelCompare<Arc>() );
    /// If the original translation lattice already contains DRs/OOVs, we have the full information.
    /// Therefore a direct composition should be enough (and more efficient).
    LINFO ( "Apply " << d_->filters.size() << " filters to the search space!" );
    for ( unsigned k = 0; k < d_->filters.size(); ++k ) {
      LDBG_EXECUTE ( fst::FstWrite ( * (d_->filters[k]), "fsts/filter.fst.gz" ) );
      LDBG_EXECUTE ( fst::FstWrite ( *fst, "fsts/before-composition.fst.gz" ) );

      if (!hipdtmode_ || pdtparens_.empty() ) {
        LINFO ("FST composition with filter");
        *fst = (fst::ComposeFst<Arc> (*fst, *d_->filters[k]) );
      } else {
        LINFO ("PDT composition");
#if OPENFSTVERSION>=1003003
        fst::PdtComposeFstOptions<Arc> opts (*fst, pdtparens_, *d_->filters[k]);
#else
        fst::PdtComposeOptions<Arc> opts (*fst, pdtparens_, *d_->filters[k]);
#endif
        opts.gc_limit = 0;
        *fst = (fst::ComposeFst<Arc> (*fst, *d_->filters[k], opts) );
      }
      LINFO ( "After filter " << k << ", NS=" << fst->NumStates() );
      Connect ( fst );
      LDBG_EXECUTE ( fst::FstWrite ( *fst, "fsts/after-composition.fst.gz" ) );
      if ( !fst->NumStates() ) break;
    }
  };

  typedef fst::ApplyLanguageModelOnTheFlyInterface<Arc> ApplyLanguageModelOnTheFlyInterfaceType;
  typedef boost::shared_ptr<ApplyLanguageModelOnTheFlyInterfaceType> ApplyLanguageModelOnTheFlyInterfacePtrType;
  std::vector<ApplyLanguageModelOnTheFlyInterfacePtrType> almotfLocal_;
  std::vector<ApplyLanguageModelOnTheFlyInterfacePtrType> almotf_;

  // Prepares language model application handlers for each kenlm type.
  // i.e. an array of templated instances of ApplyLanguageModelOnTheFly
  // Note: possibly can be refactored/merged with method initializeLanguageModelHandlers
  // in task.applylm.hpp
  template< template<class> class MakeWeightT>
  void initializeLanguageModelHandlers(const std::string& lmkey
				       , MakeWeightT<Arc> &mw
				       , std::vector<ApplyLanguageModelOnTheFlyInterfacePtrType> &almotf) {
    if (almotf.size()) {
      LINFO("Skipping!");
      return; // already done
    }
    almotf.resize(d_->klm[lmkey].size());
    unordered_set<Label> epsilons;
    for ( unsigned k = 0; k < d_->klm[lmkey].size(); ++k ) {
      USER_CHECK ( d_->klm[lmkey][k]->model != NULL,
		   "Language model " << k << " not available!" );
      almotf[k].reset(fsttools::assignKenLmHandler<Arc, MakeWeightT >(rg_, lmkey, epsilons
								      , *(d_->klm[lmkey][k])
								      , mw, true,k));	       
      mw.update();
    }
    LINFO("Initialized " << d_->klm[lmkey].size() << " language model handlers");
  }

  // \todo Merge/refactor this code with task.applylm.hpp.
  template< template<class> class MakeWeightT>
  inline fst::VectorFst<Arc> *applyLanguageModel ( const fst::Fst<Arc>& localfst
                                                   , const std::string& lmkey
                                                   , MakeWeightT<Arc> &mw
						   , std::vector<ApplyLanguageModelOnTheFlyInterfacePtrType> &almo
                                                   ) {
    if ( d_->klm.find ( lmkey ) == d_->klm.end() ) {
      if (!warned_) {
        FORCELINFO ( "No Language models for key=" << lmkey
                     <<  " available! Skipping language model application. " );
      }
      warned_ = true;
      return NULL;
    }
    
    fst::VectorFst<Arc> *output
      = new fst::VectorFst<Arc> (* (const_cast<fst::Fst<Arc> *> ( &localfst ) ) );

    // unfortunately they can be lattice-specific (pdt parentheses)
    unordered_set<Label> epsilons;
    epsilons.insert ( DR );
    epsilons.insert ( OOV );
    epsilons.insert ( EPSILON );
    epsilons.insert ( SEP );
    // If it is a pdt, add all parentheses so they get treated as epsilons too
    // for this particular lattice
    for (unsigned j = 0; j < pdtparens_.size(); ++j) {
      epsilons.insert (pdtparens_[j].first);
      epsilons.insert (pdtparens_[j].second);
    }

    for ( unsigned k = 0; k < d_->klm[lmkey].size(); ++k ) {
      LINFO ( "Composing with " << k << "-th language model" );
      d_->stats->setTimeStart ( "on-the-fly-composition "
                                +  ucam::util::toString ( k ) );
      fst::VectorFst<Arc> *aux = almo[k]->run(*output, epsilons);
      if ( !aux ) {
        LERROR ("Something very wrong happened in composition with the lm...");
        exit (EXIT_FAILURE);
      }
      delete output; output = aux;
      d_->stats->setTimeEnd ( "on-the-fly-composition "
                              + ucam::util::toString ( k ) );
      LDEBUG ( "After applying language model, NS=" <<  output->NumStates() );
    }
    LINFO ( "Connect!" );
    Connect (output);
    LINFO ( "Done! NS=" <<  output->NumStates()  );
    return output;
  }

  /**
   * \brief Applies the language model (Full translation lattice!). Currently applies on-the-fly the language model using kenlm.
   * \param localfst: lattice to score with the language model.
   */
  inline fst::VectorFst<Arc> *applyLanguageModel ( const fst::Fst<Arc>& localfst
                                                   , bool local = false ) {
    if ( local ) {
      MakeWeightHifstLocalLm<Arc> mw(rg_);
      initializeLanguageModelHandlers(locallmkey_, mw, almotfLocal_);
      if (!almotfLocal_.size()) return NULL;
      LINFO ( "Composing with local lm for inadmissible pruning (unless on top cell)" );
      return applyLanguageModel (localfst, locallmkey_, mw, almotfLocal_);
    } else {
      fst::MakeWeight<Arc> mw;
      initializeLanguageModelHandlers(lmkey_, mw, almotf_);
      if (!almotf_.size()) return NULL;
      LINFO ( "Composing with full lm for admissible pruning" );
      return applyLanguageModel (localfst, lmkey_, mw, almotf_);
    }
  };

  inline fst::VectorFst<Arc> *expand ( const fst::VectorFst<Arc>& localfst,
                                       unsigned cc, unsigned x, unsigned y ) {
    Label hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    USER_CHECK ( localfst.NumStates() > 0, "Empty lattice?" );
    ///This guy might not exist in the pair list<index,fst> , therefore we need to add it first
    if ( hieroindexexistence_.find ( hieroindex ) == hieroindexexistence_.end() )
      pairlabelfsts_.push_back ( pair< Label, const fst::Fst<Arc> * > ( hieroindex,
                                 &localfst ) );
    fst::VectorFst<Arc> *aux = new fst::VectorFst<Arc>;
    if (!hipdtmode_ ) {
      LINFO ("Replace (RTN->FSA)");
      d_->stats->setTimeStart ("replace-rtn");
      Replace (pairlabelfsts_, aux, hieroindex, !aligner_);
      d_->stats->setTimeEnd ("replace-rtn");
    } else {
      LINFO ("Replace (RTN->PDA)");
      d_->stats->setTimeStart ("replace-pdt");
      Replace (pairlabelfsts_, aux, &pdtparens_, hieroindex);
      d_->stats->setTimeEnd ("replace-pdt");
      LINFO ("Number of pdtparens=" << pdtparens_.size() );
    }
    //if it doesn't exist, then leave the pair list as it was!
    if ( hieroindexexistence_.find ( hieroindex ) == hieroindexexistence_.end() )
      pairlabelfsts_.pop_back();
    return aux;
  }

  /**
   * \remarks     Performs local pruning. It depends on the category (cc), the span (y+1) and number of states of the expanded fst.
   *              If it qualifies for local pruning, then compose fst with grammar, prune and remove grammar.
   *              Finally, lattice is reduced with standard fst operations.
   * \param       localfst: fst to be pruned.
   * \param       cc: category axis of the grid.
   * \param       x: horizontal axis of the grid
   * \param       y: vertical axis of the grid
   * \param       numstates: Estimated number of states of the expanded RTN.
   * \retval      Pointer to the pruned fst, or NULL.
   */

  fst::VectorFst<Arc> *localPruning ( const fst::VectorFst<Arc>& fst, unsigned cc,
                                      unsigned x, unsigned y ) {
#ifdef PRINTDEBUG
    std::ostringstream o;
    o << cc << "." << x << "." << y;
#endif
    if ( !localprune_ ) return NULL;
    float weight;
    unsigned referenceminstates;
    LDEBUG ( "AT " << cc << "," << x << "," << y <<
             ": Testing conditions; expected lattice size=" <<  ( *rtnnumstates_ ) ( cc, x,
                 y ) );
    if ( lpc_ ( cc, y + 1, ( *rtnnumstates_ ) ( cc, x, y ), weight ) ) {
      LINFO ( "AT " << cc << "," << x << "," << y <<
              ": Qualifies for local pruning. Making it so!" );
      LDEBUG ( "AT " << cc << "," << x << "," << y << ": expanding RTN/RmEpsilon" );
      fst::VectorFst<Arc> *efst = expand ( fst, cc, x, y );
      fst::RmEpsilon<Arc> ( efst );
      LINFO ( "AT " << cc << "," << x << "," << y << ": NS=" << efst->NumStates() );
      ++piscount_;
      LINFO("Apply filtering");
      applyFilters ( efst );
      LINFO ( "Apply LM" );
      fst::VectorFst<Arc> * latlm = applyLanguageModel ( *efst , true );

      if ( latlm != NULL ) {
        delete efst;
        //\todo Include union with shortest path...
        if (!hipdtmode_ || pdtparens_.empty() ) {
          LINFO ( "Prune with weight=" << weight );
          fst::Prune<Arc> ( latlm, mw_ ( weight ) );
        } else {
          LINFO ( "PDT expanding with weight=" << weight );
          fst::ExpandOptions<Arc> eopts (true, false, mw_ ( weight ) );
          fst::VectorFst<Arc> latlmaux;
          Expand ( *latlm, pdtparens_, &latlmaux, eopts);
          *latlm = latlmaux;
          pdtparens_.clear();
        }
        LINFO ( "Delete LM scores" );
        //Deletes LM scores if using lexstdarc or tuplearc
        //        fst::MakeWeight2<Arc> mwcopy;
        MakeWeightHifstLocalLm<Arc > mwcopy(rg_);
        fst::Map<Arc> ( latlm,
                        fst::GenericWeightAutoMapper<Arc, MakeWeightHifstLocalLm<Arc> > ( mwcopy ) );
        LINFO ( "AT " << cc << "," << x << "," << y << ": pruned with weight=" << weight
                << ",NS=" << latlm->NumStates() );
        return latlm;
      }
      LINFO ( "AT " << cc << "," << x << "," << y <<
              "Local LM not applied, filtered with " << d_->filters.size() <<
              " filter(s) ,NS=" << efst->NumStates() );
      return efst;
    }
    LINFO ( "AT " << cc << "," << x << "," << y <<
            ": Does not qualify for local pruning. " );
    return NULL;
  };

  ZDISALLOW_COPY_AND_ASSIGN ( HiFSTTask );

};

}
}  // end namespaces

#endif
