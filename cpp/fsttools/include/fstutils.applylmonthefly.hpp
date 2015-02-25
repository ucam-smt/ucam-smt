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

// nplm is providing by default
// natural logs, so correct that
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
	w = 0; // set up correctly the next state :)
	for (int k = 0; k <= NPLM_MAX_ORDER; ++k) {
	  ns.words[k]=1;
	}
      }
    }
  }
};



template<class ArcT>
struct ApplyLanguageModelOnTheFlyInterface {
  virtual VectorFst<ArcT> *run(VectorFst<ArcT> const& fst) = 0;
  virtual VectorFst<ArcT> *run(VectorFst<ArcT> const& fst, unordered_set<typename ArcT::Label> const &epsilons) = 0;
  virtual ~ApplyLanguageModelOnTheFlyInterface(){}
};

/**
 * \brief Class that applies language model on the fly using kenlm.
 * \remarks This implementation could be optimized a little further, i.e.
 * all visited states must be tracked down so that non-topsorted or cyclic fsts work correctly.
 * But we could keep track of these states in a memory efficient way (i.e. only highest state n for consecutive 0-to-n seen).
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
  unordered_map<uint64_t, pair<StateId, typename KenLMModelT::State > > statemap_;
  /// history, lmstate

  unordered_map<basic_string<unsigned>
		, StateId
		, ucam::util::hashfvecuint
		, ucam::util::hasheqvecuint> seenlmstates_;

  /// Queue of states of the new machine to process.
  queue<StateId> qc_;

  ///Arc labels to be treated as epsilons, i.e. transparent to the language model.
  unordered_set<Label> epsilons_;

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
   * \param fst         Machine you want to apply the language to. Pass a delayed machine if you can, as it will expand it in constructor.
   * \param model       A KenLM language model
   * \param epsilons    List of words to work as epsilons
   * \param natlog      Use or not natural logs
   * \param lmscale      Language model scale
   */
  ApplyLanguageModelOnTheFly ( KenLMModelT& model
                               , unordered_set<Label>& epsilons
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
    sh_.setLength(lmmodel_.Order());
    buffersize = (lmmodel_.Order() - 1 ) * sizeof (unsigned);
    buffer = const_cast<unsigned *> ( history.c_str() );
  }

  ///Destructor
  ~ApplyLanguageModelOnTheFly() {};

  virtual VectorFst<Arc> *run(const VectorFst<Arc>& fst) {
    return this->operator()(fst);
  }
  virtual VectorFst<Arc> *run(const VectorFst<Arc>& fst, unordered_set<Label> const &epsilons) {
    epsilons_ = epsilons;    // this may be necessary e.g. for pdts
    return this->operator()(fst);
  }
  ///functor:  Run composition itself
  VectorFst<Arc> * operator() (const VectorFst<Arc>& fst) {
    const VectorFst<Arc> &fst_ = fst;
    if (!fst_.NumStates() ) {
      LWARN ("Empty lattice. ... Skipping LM application!");
      return NULL;
    }

    VectorFst<Arc> *composed;    
    composed = new VectorFst<Arc>;
    ///Initialize and push with first state
    typename KenLMModelT::State bs = lmmodel_.NullContextState();
    pair<StateId, bool> nextp = add ( composed, bs, fst_.Start(), fst_.Final ( fst_.Start() ) );
    qc_.push ( nextp.first );
    composed->SetStart ( nextp.first );
    while ( qc_.size() ) {
      StateId s = qc_.front();
      qc_.pop();
      pair<StateId, const typename KenLMModelT::State> p = get ( s );
      StateId& s1 = p.first;
      const typename KenLMModelT::State s2 = p.second;

      for ( ArcIterator< VectorFst<Arc> > arc1 ( fst_, s1 ); !arc1.Done();
            arc1.Next() ) {
        const Arc& a1 = arc1.Value();
        float w = 0;
        float wp = wp_;
        typename KenLMModelT::State nextlmstate;
        if ( epsilons_.find ( a1.olabel ) == epsilons_.end() ) {
          w = lmmodel_.Score ( s2, idbridge_.map (a1.olabel), nextlmstate ) * natlog10_();
	  LDEBUG("Mapped a1.olabel=" << a1.olabel  << " to "
		<< idbridge_.map (a1.olabel)
		<< ", score=" << w);	  
	  hs_(w,wp,a1.olabel, nextlmstate); //hack to make it srilm/nplm compliant
        } else {
          nextlmstate = s2;
          wp = 0; //We don't count epsilon labels
         }
        pair<StateId, bool> nextp = add ( composed, nextlmstate
                                          , a1.nextstate
                                          , fst_.Final ( a1.nextstate ) );
        StateId& newstate = nextp.first;
        bool visited = nextp.second;
        composed->AddArc ( s
                            , Arc ( a1.ilabel, a1.olabel
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

 private:

  /**
   * \brief Adds a state.
   * \return true if the state requested has already been visited, false otherwise.
   */
  inline pair <StateId, bool> add ( fst::VectorFst<Arc> *composed, typename KenLMModelT::State& m2nextstate,
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
        pair<StateId, const typename KenLMModelT::State > ( m1nextstate, m2nextstate );
      composed->AddState();
      if ( m1stateweight != mw_ ( ZPosInfinity() ) ) composed->SetFinal (
          composed->NumStates() - 1, m1stateweight );
      stateexistence_[compound] = composed->NumStates() - 1;
      return pair<StateId, bool> ( composed->NumStates() - 1, false );
    }
    return pair<StateId, bool> ( stateexistence_[compound], true );
  };

  /**
   * \brief Get an id string representing the history, given a kenlm state.
   *
   */
  inline void getIdx ( const typename KenLMModelT::State& state,
                       uint order = 4 ) {
    memcpy ( buffer, state.words, buffersize );
    //    for ( uint k = state.length; k < history.size(); ++k ) history[k] = 0;
    for ( uint k = sh_.getLength(state); k < history.size(); ++k ) history[k] = 0;

  };

  ///Map from output state to input lattice + language model state
  inline pair<StateId, typename KenLMModelT::State > get ( StateId state ) {
    return statemap_[state];
  };

};

} // end namespaces

#endif

