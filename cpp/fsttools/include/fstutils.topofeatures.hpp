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
 * \brief Support for Topological Features. See Iglesias et al. 2015.
 * \date 10-5-2015
 * \author Gonzalo Iglesias
 */

#pragma once

#include <set>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <fstutils.hpp>

namespace ucam {
namespace fsttools {

// Tuple weights can have repeated components per arc
// Use this struct in fst::Map to merge them.
struct MergeFeatures {
  typedef TupleArc32 Arc;
  Arc operator()(Arc const &arc) const {
    if ( arc.weight == Arc::Weight::Zero() ) //irrelevant labels
      return Arc ( arc.ilabel, arc.olabel, Arc::Weight::Zero(), arc.nextstate );
    int index = 0;
    std::map<int, float> ws;
    for (fst::SparseTupleWeightIterator<FeatureWeight32, int> it ( arc.weight )
             ; !it.Done()
             ; it.Next() ) {
      ws[it.Value().first] += it.Value().second.Value();
    }
    Arc::Weight w;
    for (std::map<int,float>::const_iterator itx = ws.begin()
             ; itx != ws.end()
             ; ++itx) {
      w.Push(itx->first, itx->second);
    }
    return Arc(arc.ilabel,arc.olabel, w, arc.nextstate);
  }
};

// Used together with fst::map to map output labels into topological features
// Negative indices are reserved for topological feature and start at -2
// due to a quirk with the sparse tuple vector semiring.
struct OLabelToFeature {
  typedef fst::StdArc FromArc;
  typedef TupleArc32 ToArc;
  typedef ToArc::Weight ToWeight;
  mutable unsigned counter_;
  std::vector<unsigned> &features_;
  bool replaceOlabelByTopologicalLabel_;

  explicit OLabelToFeature(std::vector<unsigned> &features, unsigned size
                           , bool replaceOlabelByTopologicalLabel=false)
      : counter_(2) // needs this offset to work properly
      , features_(features)
      , replaceOlabelByTopologicalLabel_(replaceOlabelByTopologicalLabel)
  {
    features_.resize(size + 2,-1);
    features_[0] = 0;
  }
  void reset() { counter_ = 2;}
  ToArc operator()(FromArc const &arc) const {
    if ( arc.weight == FromArc::Weight::Zero() ) //irrelevant labels
      return ToArc ( arc.ilabel, arc.olabel, ToArc::Weight::Zero(), arc.nextstate );
    ToWeight w;
    w.Push(1, arc.weight.Value());
    LDEBUG("counter_=" << counter_
              << ",arc.olabel=" << arc.olabel
              << ",arc.weight=" << arc.weight.Value()
              << ",arc.nextstate=" << arc.nextstate
           );
    if (arc.nextstate > -1) {
      features_[counter_] = arc.olabel;
      w.Push(-counter_,1);
      LDEBUG("***olabel as feature:" << counter_ << "=>" << features_[counter_]);
      ToArc::Label ilabel =(arc.ilabel != EPSILON)? arc.ilabel: PROTECTEDEPSILON;
      ToArc::Label olabel = (replaceOlabelByTopologicalLabel_)? counter_ : ilabel;
      ++counter_;
      return ToArc ( ilabel, olabel ,w, arc.nextstate );
    }
    // final state -- irrelevant labels
    return ToArc ( arc.ilabel, arc.olabel,w, arc.nextstate );
  }

  // Go back to StdArc restoring the label
  // Assumes there is only one topological feature.
  FromArc operator()(ToArc const &arc) const {
    if ( arc.weight == ToArc::Weight::Zero() ) //irrelevant labels
      return FromArc ( arc.ilabel, arc.olabel, FromArc::Weight::Zero(), arc.nextstate );

    // assuming i only have two.
    int index = 0;
    FromArc::Weight w; w = 0;
    for (fst::SparseTupleWeightIterator<FeatureWeight32, int> it ( arc.weight )
             ; !it.Done()
             ; it.Next() ) {
      LDEBUG(it.Value().first << ":" << it.Value().second);
      it.Value().first < 0 ? index = -it.Value().first: w = it.Value().second;
    }
    if (arc.nextstate != -1) {
      LDEBUG("index=" << index << ", arc.ilabel=" << arc.ilabel
             << ", new output label = " << features_[index] );
      FromArc::Label ilabel =(arc.ilabel != PROTECTEDEPSILON)? arc.ilabel: EPSILON;
      FromArc::Label olabel = features_[index];
      return FromArc(ilabel,olabel, w, arc.nextstate);
    }
    // final state -- irrelevant labels
    return FromArc ( arc.ilabel, arc.olabel ,w, arc.nextstate );
  }
};

// for debugging:
inline std::string printState(std::map<int,int> &currentState) {
  std::stringstream ss;
  for (std::map<int,int>::iterator itx = currentState.begin()
           ; itx != currentState.end()
           ; ++itx)
    ss << itx->first << "=" << itx->second << ",";
  return ss.str();
};

// Trivial function to check whether a state is final or not.
template<class ArcT>
inline bool isFinal(fst::VectorFst<ArcT> const &fst, unsigned s) {
  if (fst.Final (s) != ArcT::Weight::Zero() ) return true;
  return false;
}

// Adds and tracks states as pairs of
// <new_state_id, (original_state_id,residual_features) >
// Note that (original_state_id,residual_features) is a variable length vector
struct StateHandler {
  // negative integers are features, positive (only one)
  // is the state of the original fst
  typedef unsigned StateId;
  // StateId, new state, final
  typedef std::pair<bool, bool> StateDetailType;
  typedef std::pair<StateId, StateDetailType  > AddResultType;

  typedef std::map<int,int> VectorState;
  typedef boost::bimap<StateId,VectorState > BimapStateIdVectorStateType;
  BimapStateIdVectorStateType state;

  fst::VectorFst<TupleArc32> &f_;
  fst::VectorFst<TupleArc32> &o_;

  explicit StateHandler(fst::VectorFst<TupleArc32> &f
                        , fst::VectorFst<TupleArc32> &o)
    : f_(f), o_(o)
  {}
  // add new state (if not already in)
  // and return state id with additional information
  AddResultType add(VectorState &out) {
    StateId in;
    if (get(out, in)) { // if exists, return id
      LDEBUG("******************* This state already exists! s="
             << in << ", vs=" << printState(out));
      return AddResultType(in,StateDetailType(false, isFinal(f_, in) ) );
    }
    in = f_.AddState();
    f_.SetFinal(in, o_.Final(out.rbegin()->first ));

    LDEBUG("******************* Adding new state " << in << ":" << printState(out));
    state.insert(BimapStateIdVectorStateType::value_type(in, out ));
    return AddResultType(in,StateDetailType(true, isFinal(f_,in)));
  }

  // return vector state given state id
  bool get(StateId const& in, VectorState &out) {
    typedef BimapStateIdVectorStateType::left_map::const_iterator Itx;
    Itx x =state.left.find(in);
    if (x != state.left.end()) {
      out = x->second;
      return true;
    }
    return false;
  }

  // return state id given vector state
  bool get(VectorState  const& in, StateId &out) {
    typedef BimapStateIdVectorStateType::right_map::const_iterator Itx;
    Itx x =state.right.find(in);
    if (x != state.right.end()) {
      out = x->second;
      return true;
    }
    return false;
  }
};

// Cursor that abstracts how we traverse
// topological features std::map with forward iterators
struct MapCursorRev {
  explicit MapCursorRev(StateHandler::VectorState &map)
      : map_(map)
      , itx_(map_.begin())
  {};
  //  void begin() { itx_ = map_.begin();};
  bool done() { return (itx_ == map_.end());};
  bool next() { ++itx_; return done();};
  StateHandler::VectorState::iterator &operator()() { return itx_; };
  StateHandler::VectorState &map_;
  StateHandler::VectorState::iterator itx_;
};

// This cursor traverses the topological features
// in reverse order. This is useful for the second pass
// over the lattice.
struct MapCursor {
  MapCursor(StateHandler::VectorState &map)
      : map_(map)
      , itx_(map_.end())
      , done_(false)
  {
    if (itx_ == map_.begin() ) done_ = true;
    else --itx_;
  }

  bool done() {
    if (done_) return true;
    if (itx_ == map_.begin()) done_ = true;
    return false;
  };
  bool next() { --itx_; return done();};
  StateHandler::VectorState::iterator &operator()() {
    return itx_;
  };

  StateHandler::VectorState &map_;
  StateHandler::VectorState::iterator itx_;
  bool done_;
};

// Tracks features for first pass expansion
class FeatureTrackerRev {
  std::map<unsigned, unsigned> trackLastFeature_;
  int const null_;
  int local_;
  bool stop_;
 public:
  FeatureTrackerRev()
      : null_(std::numeric_limits<int>::max())
      , local_(null_)
      , stop_(false)
  {};
  bool remove(unsigned s) {
    trackLastFeature_.erase(s);
  }
  int operator() (unsigned s) {
    std::map<unsigned, unsigned>::iterator itx
        = trackLastFeature_.find(s);
    if ( itx != trackLastFeature_.end()) return itx->second;
    return null_;
  }

  void operator() ( unsigned s, unsigned f) {
    std::map<unsigned, unsigned>::iterator itx
        = trackLastFeature_.find(s);
    if (itx  != trackLastFeature_.end()) {
      if (itx->second > f) itx->second = f;
    } else trackLastFeature_[s] = f;
  }
  // local updates
  void update(unsigned s) {
    LDEBUG("Updating state " << s << " with local=" << local_);
    this->operator()(s, local_);
    local_ = null_;
  }
  bool stop() {
    return stop_;
  }
  bool validate(unsigned s, unsigned k, float v) {
    stop_=false;
    int f = this->operator()(s); // lowest feature seen up to state s.
    LDEBUG("u=" << s << ": highest feature = "
           << f << ", current local feature=" << local_ << ",v=" << v);
    LDEBUG("u=" << s << ": update local:"
           << (int) local_ << ", and compare to candidate topological feature: "
           << (int) k);
    if ((int) k >= f ) { // we have a past valid feature ...
      if ( v > 0 ) { // and positive. Drop!
        LDEBUG("u=" << s << ": past feature, drop TRUE "
               << (int) k << ">=" << (int) f);
        return true;
      }
      return false;
    } else if ( v > 0 ) { // valid future feature ( k < f)
      if (local_ > (int) k ) {
        local_ = k;
        LDEBUG("u=" << s << ": update local (valid future feature):" << local_);
      }
      LDEBUG("u=" << s << ": TRUE");
      stop_=true;
      return true;
    }
    // should not be dropped:
    LDEBUG("u=" << s << " FALSE");
    return false;
  }
};

// Tracks features for second pass expansion
// TODO: refactor and clean up;
// classes FeatureTracker and FeatureTrackerRev are almost identical
class FeatureTracker {
  std::map<unsigned, unsigned> trackLastFeature_;
  int const null_;
  int local_;
  bool stop_;
 public:
  FeatureTracker()
      : null_(std::numeric_limits<int>::min())
      , local_(null_)
      , stop_(false)
  {};
  bool remove(unsigned s) {
    trackLastFeature_.erase(s);
  }
  int operator() (unsigned s) {
    std::map<unsigned, unsigned>::iterator itx
        = trackLastFeature_.find(s);
    if ( itx != trackLastFeature_.end()) return itx->second;
    return null_;
  }
  void operator() ( unsigned s, unsigned f) {
    std::map<unsigned, unsigned>::iterator itx
        = trackLastFeature_.find(s);
    if (itx  != trackLastFeature_.end()) {
      // going from lower to higher,
      // we need to see the biggest feature index.
      if (itx->second < f) itx->second = f;
    } else trackLastFeature_[s] = f;
  }
  // local updates
  void update(unsigned s) {
    LDEBUG("Updating state " << s << " with local=" << local_);
    this->operator()(s, local_);
    local_ = null_;
  }
  bool stop() {
    return stop_;
  }
  bool validate(unsigned s, unsigned k, float v) {
    stop_=false;
    int f = this->operator()(s); // lowest feature seen up to state s.
    LDEBUG("u=" << s << ": highest feature = "
           << f << ", current local feature=" << local_ << ",v=" << v);
    LDEBUG("u=" << s << ": update local:"
           << (int) local_ << ", and compare to " << (int) k);
    if ((int) k <= f ) { // we have a past valid feature ...
      if ( v > 0 ) { // and positive. Drop!
        LDEBUG("u=" << s << ": past feature, drop TRUE "
               << (int) f << "<=" << (int) k);
        return true;
      }
      if (k == f) return true; // we know for sure this has happened.
      return false;
    } else if (v > 0 ) { // valid future feature ( k < f)
      if (local_ < (int) k ) {
        local_ = k;
        LDEBUG("u=" << s << ": update local (valid future feature):" << local_);
      }
      LDEBUG("u=" << s << ": TRUE");
      stop_=true;
      return true;
    }
    // should not be dropped:
    LDEBUG("u=" << s << " FALSE");
    return false;
  }
};

template<class MapCursorT, class FeatureTrackerT >
class ExpandPathsWithTopologicalWeights {
 private:
  bool fail_;
  std::string suffix_;
  unsigned u_;
  int s_;
  fst::VectorFst<TupleArc32> out;
    // track lowest (highest) feature seen per state.
  FeatureTrackerT tracker_;
 public:
  ExpandPathsWithTopologicalWeights()
      : fail_(false)
      , s_(0), u_(0)
  {}
  bool getFail() { return fail_;}
  // TODO: mapped to super final.
  void operator()(fst::VectorFst<TupleArc32> &myfst, bool reverse) {
    using namespace fst;
    typedef TupleArc32::StateId StateId;
    std::queue < unsigned > tf; // simple fifo queue
    VectorFst<TupleArc32> aux;
    std::string suf;
    if (reverse) {
      // TODO: need method to reverse the lattice without creating extra epsilons.
      Reverse(myfst, &aux);
      suffix_ = "-reversed";
    } else aux = myfst;
    LDBG_EXECUTE(aux.Write("xx-determinized" + suffix_ + ".fst"));
    out.DeleteStates();

    LDEBUG("Start state=" << aux.Start() );
    // initialize old states? Probably not needed. Just initialize state 0
    StateHandler::VectorState empty; // 0 and bigger are normal states.
    empty[aux.Start()]=1;
    StateHandler sh(out, aux); // keeps track of state mapping
    unsigned start =sh.add(empty).first;
    LDEBUG("New start state=" << start );
    tf.push(start); // always push, added state 0.
    out.SetStart(start);

    while (tf.size()) {
      u_ = tf.front();
      StateHandler::VectorState currentState;
      if (!sh.get(u_, currentState)) {
        LDEBUG("STATE " << u_ << ": does not exist!");
        exit(EXIT_FAILURE);
      }
      // last element is the actual state of the original fst.
      s_ = currentState.rbegin()->first;
      LDEBUG("STATE " << u_ << ":" << printState(currentState));
      tf.pop();
      for ( MutableArcIterator< VectorFst<TupleArc32> > ai ( &aux, s_ )
                ; !ai.Done()
                ; ai.Next() ) {
        TupleArc32 arc = ai.Value();
        StateHandler::VectorState ns = currentState;
        LDEBUG("STATE " << u_ << ": original => s=" << s_ << ",arc="
               << arc.ilabel << "," << arc.olabel << ","
               << arc.nextstate << "w=" << ai.Value().weight);
        unsigned g=(StateId) addWeight(tf, sh, arc, ns, u_, s_);
        arc.nextstate = g; // just modify the next state
        LDEBUG("STATE " << u_ << ": after => s=" << s_ << ",arc="
               << arc.ilabel << "," << arc.olabel << ","
               << arc.nextstate << "w=" << ai.Value().weight);
        out.AddArc(u_, arc); // add arc, weight has been modified.
      }
      tracker_.remove(u_);
      LDEBUG("STATE " << u_ << ": Exit the arc loop");
    }
    LDEBUG("Finished! Reverse back and topsort");
    LDBG_EXECUTE(out.Write("xx-determinized-topo-expanded" + suffix_ + ".fst"));
    if (reverse) Reverse(out, &myfst);
    else myfst = out;
    TopSort(&myfst);  // show it topologically sorted
  };

 private:
  // Deletes current state from the vector state, removes 0-valued features,
  // determines any arc weight contribution present on the vector state
  void purge(StateHandler::VectorState &nf, TupleArc32 &arc, TupleArc32::Weight &w) {
    StateHandler::VectorState::iterator itx = nf.end(); // doesn't work with rbegin
    --itx;
    nf.erase(itx); // delete current state -- will add new one at the end.
    LDEBUG("STATE " << u_ << ": s="
           << s_ << "BEFORE updating with features (no next state!): "
           << printState(nf));
    // for now, nf should only contain topological features.
    for (fst::SparseTupleWeightIterator<FeatureWeight32, int> it ( arc.weight )
             ; !it.Done()
             ; it.Next() ) {
      int const &k = it.Value().first;
      if (k >=0) {
        w.Push(k, it.Value().second);
        continue;
      }
      // only negative feature indices <=> topological features.
      nf[k] += it.Value().second.Value(); // +1 or -1
      if (nf[k] == 0) {
        nf.erase(k); // delete 0 values to avoid unnecessary clutter
      }
    }
  };

  // size should always be at least 1 (the current state itself).
  void assertNotEmpty(StateHandler::VectorState &nf) {
    if (!nf.size()) {
      LERROR("General error: Empty state.");
      exit(EXIT_FAILURE);
    }
  };

  void pop(StateHandler::VectorState &nf, TupleArc32 &arc, TupleArc32::Weight &w) {
    MapCursorT mc(nf);
    StateHandler::VectorState drop;
    bool yei=false;
    unsigned countNegatives = 0;
    unsigned countPositives = 0;
    while (!mc.done() ) {
      StateHandler::VectorState::iterator &itx = mc();
      LDEBUG("STATE " << u_ << ", state " << s_
             << ": let us validate feature "
             << itx->first << "=>" << itx->second);

      if (itx->first < 0 && tracker_.validate(u_, -itx->first, itx->second) ) {

        if (itx->second < 0 ) ++countNegatives;
        else if (itx->second > 0 ) ++countPositives;
        if (itx->second < 0 || countNegatives + 1 >= countPositives )
          drop[itx->first]=itx->second;
        if (tracker_.stop()) {
          LDEBUG("STATE " << u_ << ", state " << s_ << ": "
                 << itx->first << " feature found. We are done here");
          yei=true;
          break;
        }
      }
      mc.next();
    }
    if (!yei) {
      LDEBUG("STATE " << u_ << ", state " << s_ << ": did not find a feature??");
    }
    // add topological feature (s) to w
    for (StateHandler::VectorState::iterator itx2 = drop.begin()
             ; itx2 != drop.end()
             ; ++itx2
         ) {
      w.Push(itx2->first, itx2->second);
      nf.erase(itx2->first); // probably should generate residue on the fly
    }
    // we have at least one feature we wanna drop.
    // If drop.size() > 1 then we can also flag problem
    // (and will need double pass to fix).
    if (!drop.size()) {
      if (u_) {
        fail_=true;
      }

    } else if (drop.size() > 1) {
      LDEBUG("Marking  as non-solved");
      fail_=true;
    }
  };

  // Adds weight and topological feature for the current arc;
  // will add new state if it didn't exist before, using
  // StateHandler class.
  // Returns the next state id.
  // Note: ignores epsilon arcs. This is useful as
  // our lattices are assumed to start (end) with epsilons
  // (i.e. paths in reversed lattice start with an epsilon
  unsigned addWeight(std::queue < unsigned > &tf
                     , StateHandler &sh
                     , TupleArc32 &arc
                     , StateHandler::VectorState &nf
                     , unsigned u, int s) {
    assertNotEmpty(nf);
    TupleArc32::Weight w;
    purge(nf, arc, w);
    LDEBUG("STATE " << u_ << ": s=" << s_ <<", AFTER purge: " << printState(nf));
    // remove lowest feature index (or more) and store in the arc weight.
    // don't want to do this on start state for a reversed lattice :).
    if (arc.ilabel != 0) {
      pop(nf, arc, w);
    } // if the arc is an epsilon, just leave arc as is.
    arc.weight = w;
    // encode original nextstate, guaranteed to be at the end of the set.
    nf[arc.nextstate]=1;
    LDEBUG("STATE " << u_ << ": s=" << s_
           <<", AFTER pop (includes next state): " << printState(nf));
    StateHandler::AddResultType art=sh.add(nf);
    if (art.second.first) {
      LDEBUG("STATE " << u_ << ": s=" << s_ << ", Added new state "
             << art.first << ":" << printState(nf)
             << " -- goes to queue");
      tf.push( art.first); // push new state
    } else {
      LDEBUG("STATE " << u_ << ": s=" << s_ << ", Added new state "
             << art.first << ":" << printState(nf)
             << " -- DOES NOT go to queue");
    }

    if (arc.ilabel != 0)
      tracker_.update(art.first);
    // Next state is final (reversed lattice, so we only have ONE final state).
    // Furthermore, a twice reversed FSA only has one final state with epsilons.
    if (art.second.second) {
      // Check at this point, nf only contains the next state.
      if (nf.size() >  1) {
        LERROR("STATE " << u_ << "s=" << s_
               << ",At the end of the path we still have... features!"
               << printState(nf));
        // hack: delete the nextstate feature.
        nf.erase(arc.nextstate);
        for (StateHandler::VectorState::iterator itx2 = nf.begin()
                 ; itx2 != nf.end()
                 ; ++itx2
             ) {
          arc.weight.Push(itx2->first, itx2->second);
        }
        fail_ = true;
      }
    }
    return art.first;
  }
};

// Map output labels into topological features
// Runs a forward pass over the lattice. Labels are
// defined based on (label-name,partial-hyp-min-length).
// Instead of keeping one label per arc, we ensure they
// are unique at the hypothesis level.
// assumes topologically sorted ifst on tropical semiring
inline void TopologicalLabelMap(fst::VectorFst<fst::StdArc> const &ifst
                                , fst::VectorFst<TupleArc32> *ofst
                                , std::vector<unsigned> *features) {
  using namespace fst;
  typedef boost::unordered_map<unsigned, unsigned > PathCountType;
  typedef std::pair<unsigned,unsigned> VectorState;
  typedef boost::bimap<unsigned,VectorState > BimapStateIdVectorStateType;

  BimapStateIdVectorStateType mit;
  PathCountType tf;
  std::vector<unsigned> &ft = *features;
  ofst->ReserveStates(ifst.NumStates());
  for (unsigned k =0; k < ifst.NumStates(); ++k) ofst->AddState();
  ofst->SetStart(ifst.Start());
  tf[ifst.Start()] = 0; // we start with length 0.

  for ( StateIterator< VectorFst<StdArc> > si (ifst ); !si.Done(); si.Next() ) {
    unsigned state_id = si.Value();
    LDEBUG("State Id =" << state_id);
    unsigned &pcount = tf[state_id];

    if (ifst.Final(state_id) != StdArc::Weight::Zero() ) {
      TupleArc32::Weight z;
      z.Push(1,ifst.Final(state_id));
      ofst->SetFinal(state_id, z);
    }
    for ( ArcIterator< VectorFst<StdArc> > ai ( ifst, si.Value() )
              ; !ai.Done()
              ; ai.Next() ) {

      StdArc arc = ai.Value();
      tf[arc.nextstate] = std::max(tf[state_id] +1, tf[arc.nextstate]);
      typedef BimapStateIdVectorStateType::right_map::const_iterator Itx;
      std::pair<unsigned,unsigned> a(arc.olabel, pcount);
      Itx x = mit.right.find(a);
      unsigned index;
      if (x != mit.right.end()) {
        index = x->second;
      } else {
        index = mit.size() + 2;
        mit.insert(BimapStateIdVectorStateType::value_type(index, a));
      }

      ft[index ] = arc.olabel; // mapping
      LDEBUG("State Id =" << state_id << ": ft at "
             << index  << "=" << ft[index ]);
      TupleArc32::Weight y;
      y.Push(1, arc.weight);
      y.Push(-index, 1);
      TupleArc32 ab(arc.ilabel, arc.ilabel, y,arc.nextstate);
      ofst->AddArc(state_id,ab);
    }
    tf.erase(state_id); // we are done with this state;
  }
};

// Actions (typically standard sequence of fst operations).
// See TopoFeaturesHelper class below
struct ProjectDeterminizeMinimizePushAction {
  void operator()(fst::VectorFst<TupleArc32> const &in
                  , fst::VectorFst<TupleArc32> *out) {
    using namespace fst;
    LDBG_EXECUTE(in.Write("noaction.fst"));
    Determinize(ProjectFst<TupleArc32>(in,PROJECT_INPUT), out);
    LDBG_EXECUTE(out->Write("determinized.fst"));
    Minimize(out);
    LDBG_EXECUTE(out->Write("determinized-minimized.fst"));
    Push<TupleArc32,REWEIGHT_TO_FINAL>(*out,out,kPushWeights);
    LDBG_EXECUTE(out->Write("determinized-minimized-pushed.fst"));
  }
};

struct ProjectDeterminizeAction {
  void operator()(fst::VectorFst<TupleArc32> const &in
                  , fst::VectorFst<TupleArc32> *out) {
    using namespace fst;
    Determinize(ProjectFst<TupleArc32>(in,PROJECT_INPUT), out);
    LDBG_EXECUTE(out->Write("determinized.fst"));
  }
};

struct ProjectDeterminizePushAction {
  void operator()(fst::VectorFst<TupleArc32> const &in
                  , fst::VectorFst<TupleArc32> *out) {
    using namespace fst;
    Determinize(ProjectFst<TupleArc32>(in,PROJECT_INPUT), out);
    LDBG_EXECUTE(out->Write("determinized.fst"));
    Push<TupleArc32,REWEIGHT_TO_FINAL>(*out,out,kPushWeights);
    LDBG_EXECUTE(out->Write("determinized-minimized-pushed.fst"));
  }
};

struct NullAction {
  void operator()(fst::VectorFst<TupleArc32> const &in
                  , fst::VectorFst<TupleArc32> *out) {
    using namespace fst;
    *out = in;
    LDBG_EXECUTE(out->Write("null-action.fst"));
  }
};

/**
  * \brief A wrapper that runs maps labels to topological features,
  * runs an "action" (sequence of standard fst operations defined as
  * a policy per ActionT), and then expands the resulting lattice
  * and moves topological features
  * to allow for a 1-1 reverse mapping from features to output labels
  * on the same arcs.
  * \remark Note that it protects epsilons (ie relabel). In particular,
  * this is important for ExpandPathsWithTopologicalWeights
  */
template<class ActionT>
struct TopoFeaturesHelper {
  ActionT action_;
  bool exitOnFailure_;
  bool reverseFirst_;
  explicit TopoFeaturesHelper(bool exitOnFailure)
      : exitOnFailure_(exitOnFailure)
      , reverseFirst_(true)
  {}
  // Map output labels as <olabel, max # times it has appeared in previous paths>
  // plus use double pass if needed.
  inline void operator() (fst::VectorFst<fst::StdArc> *fst) {
    using namespace fst;
    // get total number of arcs.
    unsigned narcs = 0;
    for (unsigned k = 0; k < fst->NumStates(); ++k){
      narcs += fst->NumArcs(k);
    }
    std::vector<unsigned> features(narcs, -1);
    OLabelToFeature otf(features,features.size(), true); // could be false
    GenericArcMapper<StdArc,TupleArc32, OLabelToFeature> gam(otf);
    GenericArcMapper<TupleArc32, StdArc, OLabelToFeature> gamr(otf);
    VectorFst<TupleArc32> mfstTopoFeatures, dfst;
    VectorFst<StdArc> result;
    // RelabelUtility 0s to some special symbol.
    // this implementation skips epsilons, so in order to be
    // tagged properly they must be protected.
    RelabelUtil<StdArc>().addIPL(EPSILON,PROTECTEDEPSILON)(&*fst);
    LDBG_EXECUTE(fst->Write("01-topsorted-input.th.fst"));
    //Map(*fst, &mfstTopoFeatures,gam);
    TopologicalLabelMap(*fst, &mfstTopoFeatures, &features);
    LDBG_EXECUTE(mfstTopoFeatures.Write("02-topologically-mapped.th.fst"));

    // ActionT typically performs a sequence of transformations
    // to FSAs, defined by the user.
    // It is also responsibility of the user to ensure
    // that topological features "sufficiently" pushed towards
    // end state, i.e. they appear wherever possible
    // in the reversed lattice before or at the expected arc.
    // ensure output is acyclic and epsilon-free.
    action_(mfstTopoFeatures, &dfst);
    LDBG_EXECUTE(dfst.Write("03-det-topo.th.fst"));
    // simplify features -- silly hack motivated by underlying
    // sparse tuple semiring representation.
    MergeFeatures mf;
    GenericArcAutoMapper<TupleArc32, MergeFeatures> gam2(mf);
    VectorFst<TupleArc32> dfstw;
    Map(dfst,&dfstw, gam2);
    // simplify features -- end of silly hack

    LDBG_EXECUTE(dfstw.Write("04-det-topo-merged.th.fst"));
    ExpandPathsWithTopologicalWeights<MapCursorRev, FeatureTrackerRev> epw;
    epw(dfstw, reverseFirst_); // determinized lattice, ergo we start reversed.
    LDBG_EXECUTE(dfstw.Write("05-det-topo-expanded-rev.th.fst"));
    if (epw.getFail()) {
      if (exitOnFailure_) {
        LERROR("Failed to position correctly all topological features in ONE pass.");
        exit(EXIT_FAILURE);
      }
      FORCELINFO("First pass did not position all features correctly."
                 << " Running Second pass:");
      // double reverse, now have an epsilon arc from start state,
      // and epsilon to single final state.
      ExpandPathsWithTopologicalWeights<MapCursor, FeatureTracker> epw2;
      epw2(dfstw, !reverseFirst_);
      LDBG_EXECUTE(dfstw.Write("06-det-topo-expanded.th.fst"));
      if (epw2.getFail()) {
        LERROR("Failed to position correctly all topological features"
               << " after the second pass");
        exit(EXIT_FAILURE);
      }
    }
    Map(dfstw,&result,gamr);
    RelabelUtil<StdArc>().addIPL(PROTECTEDEPSILON, EPSILON)(&*fst);
    LDBG_EXECUTE(result.Write("07-det-topo-final.th.fst"));
    fst->DeleteStates();
    *fst = result;
  };
};

}} // end namespace

