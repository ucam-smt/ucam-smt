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

#ifndef FSTUTILS_APPLYLMONTHEFLY_HPP
#define FSTUTILS_APPLYLMONTHEFLY_HPP

/**
 * \file
 * \brief Contains implementation of ApplyLanguageModelOnTheFly
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <idbridge.hpp>
#include <lm/wrappers/nplm.hh>
namespace fst {


template <class StateT>
struct StateHandler {
  inline void setLength(unsigned length) {}
  inline unsigned getLength(StateT const &state) { return state.length;}
};

// specialization to handle nplm
template<>
struct StateHandler<lm::np::State> {
  unsigned length_;
  inline void setLength(unsigned length) { length_ = length;}
  inline unsigned getLength(lm::np::State const &state) const { return length_; }
};

template<class StateT>
struct Scale {
  float scale_;
  Scale(float scale): scale_(scale) {}
  float operator()(){
    return scale_;
  }
};

// nplm in kenlm is providing by default
// natural logs, so correct this
template<>
struct Scale<lm::np::State> {
  float scale_;
  Scale(float scale): scale_(scale / ::log(10)) {}
  float operator()(){
    return scale_;
  }
};

// This silly little hack allows to make it srilm-compatible
// i.e. log10 scores should always match
// assumes internal 1/2 numbers are <s> / </s>
// if you don't care, just implement a functor that does nothing
// and use it instead of this one
template<class StateT>
struct HackScore {
  void operator()(float &w, float &wp, unsigned label, StateT&) {
    if ( label <= 2  )  {
      wp = 0;
      if (label == 1 ) w = 0; //get same result as srilm
    }
  }
};

// But nplm does not do exactly the same thing because
// the start state is actually different
template<>
struct HackScore<lm::np::State> {
  void operator()(float &w, float &wp, unsigned label, lm::np::State &ns) {
    if ( label <= 2  )  {
      wp = 0;
      if (label==1) {
	w = 0;
        // set up correctly the next state
	for (int k = 0; k < NPLM_MAX_ORDER - 1; ++k) {
	  ns.words[k]=1;
	}
      }
    }
  }
};

template<class StateT>
struct HackScoreBilingual {
  void operator()(float &w, float &wp, unsigned label, StateT&) {
    LERROR("Use only for nplm in bilingual models!");
    exit(EXIT_FAILURE);
  }
};


template<>
struct HackScoreBilingual<lm::np::State> {
  void operator()(float &w, float &wp, unsigned label, lm::np::State &ns) {
    if ( label <= 2  )  {
      wp = 0;
      if (label==1) {
	for (int k = 0; k < NPLM_MAX_ORDER - 1; ++k) {
	  ns.words[k]=1;
	}
      }
    }
  }
};

// nplm states have size NPLM_MAX_ORDER - 1
template<class StateT>
std::string printDebug(StateT const &state) {
  std::string o =ucam::util::toString<unsigned>(state.words[0]);
  for (unsigned k = 1; k < NPLM_MAX_ORDER - 1; ++k) {
    o += "," + ucam::util::toString<unsigned>(state.words[k]);
  }
  return o;
};

template<class StateT
         , class KenLMModelT
         , class IdBridgeT
         , template<class> class HackScoreT
         >
struct Scorer {
  IdBridgeT const& idbridge_;
  KenLMModelT& lmmodel_;
  HackScoreT<StateT> hs_;
  Scale<StateT> &natlog10_;
  explicit Scorer(KenLMModelT &lmmodel
                  , IdBridgeT const &idbridge
                  , Scale<StateT> &nl
                  , unsigned
                  , std::vector<std::vector<unsigned> > const &
                  )
      : idbridge_(idbridge)
      , lmmodel_(lmmodel)
      , natlog10_(nl)
  {
    //    LERROR("Bilingual model scorer only works with nplm models");
    //    exit(EXIT_FAILURE);
  }

  void operator()(StateT const &current, float &w, float &wp, int ilabel, int olabel, StateT& next) {
    w = lmmodel_.Score ( current, idbridge_.map(olabel), next ) * natlog10_();
    hs_(w, wp, olabel, next); //hack to make it srilm/nplm compliant
  }
};

// partial template specialization class for nplm model
// TODO: code reorganization/simplification needed
template< class IdBridgeT
         , template<class> class HackScoreT
         >
struct Scorer<lm::np::State, lm::np::Model
              , IdBridgeT, HackScoreT> {
  IdBridgeT const& idbridge_;
  typedef lm::np::Model  KenLMModelT;
  typedef lm::np::State  StateT;
  KenLMModelT & lmmodel_;
  HackScoreT<StateT> hs_;
  Scale<StateT> &natlog10_;
  unsigned srcSize_;

  std::vector< std::vector<unsigned> > srcWindows_;

  Scorer(KenLMModelT &lmmodel
         , IdBridgeT const &idbridge
         , Scale<StateT> &nl
         , unsigned srcSize
         , std::vector<std::vector<unsigned> > const &srcWindows
         )
      : idbridge_(idbridge)
      , lmmodel_(lmmodel)
      , natlog10_(nl)
      , srcSize_(srcSize)
  {
    srcWindows_.clear();
    // need to map source labels into nplm internal labels.
    srcWindows_.resize(srcWindows.size());
    for (unsigned k = 0; k < srcWindows.size(); ++k) {
      srcWindows_[k].resize(srcWindows[k].size());
      for (unsigned j = 0; j < srcWindows[k].size(); ++j) {
        srcWindows_[k][j] = idbridge_.map(srcWindows[k][j]);
      }
    }
#ifdef PRINTDEBUG1
    // For debugging purposes, lets print mapped vectors here:
    for (unsigned k = 0; k < srcWindows_.size(); ++k) {
      std::stringstream ss; ss << srcWindows_[k][0];
      for (unsigned j = 1; j < srcWindows_[k].size(); ++j ){
        ss << " " << srcWindows_[k][j];
      }
      LDEBUG("*** MAPPED src words=" << ss.str());
    }
#endif
  };

  // For bilingual model
  // This assumes that ilabels are indices or pointers to the source sentence
  // olabels are actual target words.
  void operator()(StateT const &current, float &w, float &wp, int ilabel
                  , int olabel, StateT& next) {
    // make a copy of current state and add source window.
    StateT c2 = current;
    LDEBUG("Current model state=" << printDebug(c2));
    if (srcWindows_.size()) {
      --ilabel;
      if (ilabel >= srcWindows_.size() || ilabel < 0) {
        LERROR("Wrong input label! Input labels should be links/affiliations "
               << "pointing to words in the source sentence:" << ilabel);
        exit(EXIT_FAILURE);
      }
#ifdef PRINTDEBUG1
      std::stringstream ss;
#endif
      for (int k = 0; k < srcSize_; ++k) {
#ifdef PRINTDEBUG1
        ss << ilabel << "," << k << ": " << srcWindows_[ilabel][k] << "\t";
#endif
        c2.words[k] = srcWindows_[ilabel][k];
      }
      LDEBUG(ss.str() << "\nilabel="
             << ilabel << ", Current model state after adding source="
             << printDebug(c2));
    }

    unsigned ol = idbridge_.mapOutput(olabel);
    w = lmmodel_.Score ( c2, ol, next ) * natlog10_();
    LDEBUG("Mapped olabel=" << olabel  << " to "
           << ol
           << ", score=" << w);

    LDEBUG("Next state is (kenlm) =" << printDebug(next));
    hs_(w, wp, olabel, next); //hack to make it srilm/nplm compliant
    LDEBUG("Next state is (after hack) =" << printDebug(next));
    // finally, update nextstate by taking current state,
    // adding olabel and sliding the window
    // (only target model context)
    // this could go into the hackscore class?

    // kenlm has shifted the state, but now i take out
    // the source words in the input
    for (int k = 0; k < srcSize_; ++k) next.words[k] = 0;
    unsigned oli = idbridge_.map(olabel);
    bool failure = false;
    int offset = (int) lmmodel_.Order() - 2;
    if (next.words[offset] != ol) {
      LERROR("Problem:" << offset << "=>" << next.words[offset]);
      failure = true;
    }
    // ... and map the last word with the input vocabulary (instead of output)
    // input and output vocabularies are not necessarily the same
    // and this is particularly true in a bilingual model!
    next.words[offset] = oli;
    LDEBUG("*** " << printDebug(c2)
           << "\t" << printDebug(revertMappings(c2))
           << "\t" << w << "\t" << printDebug(next)
           << "\t" << printDebug(revertMappings(next))
           << "\t" << olabel << "\t i=" << oli << ", o=" << ol
           );
    if (failure) exit(EXIT_FAILURE);
  }

  StateT revertMappings(StateT const &s) {
    StateT x;
    for (int k = 0; k < NPLM_MAX_ORDER; ++k) {
      x.words[k] = idbridge_.rmap(s.words[k]);
    }
    return x;
  }
};

  /**
   * \brief Interface for language model application
   * Provides different run methods to do composition
   * with a (bilingual) model
   */
template<class ArcT>
struct ApplyLanguageModelOnTheFlyInterface {
  virtual VectorFst<ArcT> *run(VectorFst<ArcT> const& fst) = 0;
  virtual VectorFst<ArcT> *run(VectorFst<ArcT> const& fst
                               , std::unordered_set<typename ArcT::Label> const &epsilons) = 0;
  virtual VectorFst<ArcT> *run(const VectorFst<ArcT>& fst
                               , unsigned srcSize
                               , std::vector< std::vector<unsigned> >  &srcWindows) =0;
  virtual ~ApplyLanguageModelOnTheFlyInterface(){}
};

/**
 * \brief Class that applies language model on the fly using kenlm.
 * \remarks This implementation could be optimized a little further, i.e.
 * all visited states must be tracked down so that non-topsorted or cyclic fsts
 * work correctly. But we could keep track of these states in a
 * memory efficient way (i.e. only highest state n for consecutive 0-to-n seen).
 */
template <class Arc
          , class MakeWeightT = MakeWeight<Arc>
          , class KenLMModelT = lm::ngram::Model
          , class IdBridgeT = ucam::fsttools::IdBridge
	  , template<class> class HackScoreT = HackScore >
class ApplyLanguageModelOnTheFly : public ApplyLanguageModelOnTheFlyInterface<Arc> {
 private:
  typedef typename Arc::StateId StateId;
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;
  typedef unsigned long long ull;

  unordered_map< ull, StateId > stateexistence_;

  static const ull sid = 1000000000;
  /// m12state=<m1state,m2state>
  unordered_map<uint64_t
                , std::pair<StateId, typename KenLMModelT::State > > statemap_;
  /// history, lmstate

  unordered_map<basic_string<unsigned>
		, StateId
		, ucam::util::hashfvecuint
		, ucam::util::hasheqvecuint> seenlmstates_;

  /// Queue of states of the new machine to process.
  queue<StateId> qc_;

  ///Arc labels to be treated as epsilons, i.e. transparent to the language model.
  std::unordered_set<Label> epsilons_;

  KenLMModelT& lmmodel_;
  const typename KenLMModelT::Vocabulary& vocab_;

  //  float  natlog10_;
  Scale<typename KenLMModelT::State> natlog10_;

  ///Templated functor that creates weights.
  MakeWeightT mw_;

  basic_string<unsigned> history;
  unsigned *buffer;
  unsigned buffersize;

  //Word Penalty.
  float wp_;

  const IdBridgeT& idbridge_;

  // transparent state handling (kenlm vs nplm)
  StateHandler<typename KenLMModelT::State> sh_;
  // transparent score quirks handling for srilm/nplm compliance
  HackScoreT<typename KenLMModelT::State> hs_;

  ///Public methods
 public:

  ///Set MakeWeight functor
  inline void setMakeWeight ( const MakeWeightT& mw ) {
    mw_ = mw;
  };

  /**
   * Constructor. Initializes on-the-fly composition with a language model.
   * \param fst         Machine you want to apply the language to.
   *                    Pass a delayed machine if you can, as it will expand
   *                    it in constructor.
   * \param model       A KenLM language model
   * \param epsilons    List of words to work as epsilons
   * \param natlog      Use or not natural logs
   * \param lmscale      Language model scale
   */
  ApplyLanguageModelOnTheFly ( KenLMModelT& model
                               , std::unordered_set<Label>& epsilons
                               , bool natlog
                               , float lmscale
                               , float lmwp
                               , const IdBridgeT& idbridge
			       , MakeWeightT &mw
			       )
    : natlog10_ ( natlog ? -lmscale* ::log ( 10.0 ) : -lmscale )
    , lmmodel_ ( model )
    , vocab_ ( model.GetVocabulary() )
    , wp_ ( lmwp )
    , epsilons_ ( epsilons )
    , history ( model.Order(), 0)
    , idbridge_ (idbridge)
    , mw_(mw)
  {
    init();
  };

  ApplyLanguageModelOnTheFly ( KenLMModelT& model
                               , bool natlog
                               , float lmscale
                               , float lmwp
                               , const IdBridgeT& idbridge
			       , MakeWeightT &mw
			       )
    : natlog10_ ( natlog ? -lmscale* ::log ( 10.0 ) : -lmscale )
    , lmmodel_ ( model )
    , vocab_ ( model.GetVocabulary() )
    , wp_ ( lmwp )
    , history ( model.Order(), 0)
    , idbridge_ (idbridge)
    , mw_(mw)
  {
    init();
  };

  void init() {
    LDEBUG("Model order=" << (int) lmmodel_.Order());
    sh_.setLength(lmmodel_.Order() );
    buffersize = (lmmodel_.Order() - 1 ) * sizeof (unsigned);
    buffer = const_cast<unsigned *> ( history.c_str() );
  }

  ///Destructor
  ~ApplyLanguageModelOnTheFly() {};

  VectorFst<Arc> *run(const VectorFst<Arc>& fst) {
    return this->operator()(fst);
  }
  VectorFst<Arc> *run(const VectorFst<Arc>& fst
                      , std::unordered_set<Label> const &epsilons) {
    epsilons_ = epsilons;    // this may be necessary e.g. for pdts
    return this->run(fst);
  }

  VectorFst<Arc> *run(const VectorFst<Arc>& fst
                      , unsigned size
                      , std::vector<std::vector<unsigned> > &srcWindows) {
    //    epsilons_ = epsilons;    // this may be necessary e.g. for pdts
    return this->operator()(fst,size, srcWindows);
  };


  ///functor:  Run composition itself. Use for target-only LMs.
  VectorFst<Arc> * operator() (const VectorFst<Arc>& fst) {
    unsigned ign = 0;
    std::vector<std::vector<unsigned> > empty;
    Scorer<typename KenLMModelT::State, KenLMModelT, IdBridgeT, HackScoreT>
      sc(lmmodel_, idbridge_, natlog10_, ign, empty );
    return doComposition(fst, sc);
  }

  // Composition with bilm
  // note that input labels are indices to source windows
  VectorFst<Arc> * operator() (const VectorFst<Arc>& fst, unsigned srcSize
                               , std::vector<std::vector<unsigned> > &srcw) {

    // the scorer will help compute the correct score regardless of whether it is a
    // bilingual model or not, etc.
    Scorer<typename KenLMModelT::State, KenLMModelT, IdBridgeT, HackScoreT>
      sc(lmmodel_, idbridge_, natlog10_ , srcSize, srcw);
    return doComposition(fst, sc);
  }

 private:

  /**
   * \brief Runs the actual composition
   * \return NULL pointer if no composition, a pointer to the resulting FST otherwise
   */
  VectorFst<Arc> * doComposition(const VectorFst<Arc>& fst
                                 , Scorer<typename KenLMModelT::State, KenLMModelT, IdBridgeT, HackScoreT> &sc) {

    if (!fst.NumStates() ) {
      LWARN ("Empty lattice. ... Skipping LM application!");
      return NULL;
    }
    VectorFst<Arc> *composed = new VectorFst<Arc>;
    ///Initialize and push with first state
    typename KenLMModelT::State bs = lmmodel_.NullContextState();
    std::pair<StateId, bool> nextp = add ( composed, bs, fst.Start(), fst.Final ( fst.Start() ) );
    qc_.push ( nextp.first );
    composed->SetStart ( nextp.first );
    while ( qc_.size() ) {
      LDEBUG("queue size=" << qc_.size());
      StateId s = qc_.front();
      qc_.pop();
      std::pair<StateId, const typename KenLMModelT::State> p = get ( s );
      StateId& s1 = p.first;
      const typename KenLMModelT::State s2 = p.second;

      for ( ArcIterator< VectorFst<Arc> > arc1 ( fst, s1 ); !arc1.Done();
            arc1.Next() ) {
        const Arc& a1 = arc1.Value();
        float w = 0;
        float wp = wp_;
        typename KenLMModelT::State nextlmstate;
        if ( epsilons_.find ( a1.olabel ) == epsilons_.end() ) {
          sc(s2, w, wp, a1.ilabel, a1.olabel, nextlmstate);
        } else {
          // ignore epsilons completely, even if we have alignments here.
          nextlmstate = s2;
          wp = 0; //We don't count epsilon labels
         }
        std::pair<StateId, bool> nextp = add ( composed, nextlmstate
                                          , a1.nextstate
                                          , fst.Final ( a1.nextstate ) );
        StateId& newstate = nextp.first;
        bool visited = nextp.second;
        composed->AddArc ( s, Arc( a1.ilabel, a1.olabel
                                   , Times ( a1.weight, Times (mw_ ( w ) , mw_ (wp) ) )
                                   , newstate ) );
        //Finally, only add newstate to the queue if it hasn't been visited previously
        if ( !visited ) {
          qc_.push ( newstate );
        }
      }
    }
    LINFO ( "Done! Number of states=" << composed->NumStates() );
    stateexistence_.clear();
    statemap_.clear();
    seenlmstates_.clear();
    history.resize( lmmodel_.Order(), 0);
    return composed;
  };

  /**
   * \brief Adds a state.
   * \return true if the state requested has already been visited, false otherwise.
   */
  inline std::pair <StateId, bool> add ( fst::VectorFst<Arc> *composed, typename KenLMModelT::State& m2nextstate,
                                    StateId m1nextstate, Weight m1stateweight ) {
    static StateId lm = 0;
    getIdx ( m2nextstate );
    ///New history:
    if ( seenlmstates_.find ( history ) == seenlmstates_.end() ) {
      seenlmstates_[history] = ++lm;
    }
    uint64_t compound = m1nextstate * sid + seenlmstates_[history];
    LDEBUG ( "compound id=" << compound );
    if ( stateexistence_.find ( compound ) == stateexistence_.end() ) {
      LDEBUG ( "New State!" );
      statemap_[composed->NumStates()] =
        std::pair<StateId, const typename KenLMModelT::State > ( m1nextstate, m2nextstate );
      composed->AddState();
      LDEBUG("Added..." << composed->NumStates() << "," << m1nextstate << "," << printDebug(m2nextstate));
      if ( m1stateweight != mw_ ( ZPosInfinity() ) ) composed->SetFinal (
          composed->NumStates() - 1, m1stateweight );
      stateexistence_[compound] = composed->NumStates() - 1;
      return std::pair<StateId, bool> ( composed->NumStates() - 1, false );
    }
    return std::pair<StateId, bool> ( stateexistence_[compound], true );
  };

  /**
   * \brief Get an id string representing the history, given a kenlm state.
   *
   */
  inline void getIdx ( const typename KenLMModelT::State& state,
                       uint order = 4 ) {
    LDEBUG("getting Idx");
    memcpy ( buffer, state.words, buffersize );
    //    for ( uint k = state.length; k < history.size(); ++k ) history[k] = 0;
        for ( unsigned k = sh_.getLength(state); k < history.size(); ++k ) history[k] = 0;

  };

  ///Map from output state to input lattice + language model state
  inline std::pair<StateId, typename KenLMModelT::State > get ( StateId state ) {
    LDEBUG("get");
    return statemap_[state];
  };

};

} // end namespaces

#endif

