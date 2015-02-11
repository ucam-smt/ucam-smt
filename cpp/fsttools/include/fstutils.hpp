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
 * \brief Utilites to extract vocabulary, pseudo-determinize lattices and build substring transducers.
 * \date 14-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef FSTUTILS_HPP
#define FSTUTILS_HPP

namespace fst {

///Just a wrapper to maintain compatibility with OpenFST 1.3.1, last version using kPosInfinity constant
inline float ZPosInfinity() {
#if OPENFSTVERSION>=1003002 //1.3.2
  return fst::FloatLimits<float>::PosInfinity();
#else
  return fst::FloatLimits<float>::kPosInfinity;   //Up to OpenFST 1.3.1
#endif
}

/**
 * \brief Extract source (left-side) vocabulary from an fst.
 * \param myfst: The fst or lattice for which we want to extract the vocabulary.
 * \param vcb: To store the vocabulary (as strings).
 */

template<class Arc >
inline void extractSourceVocabulary ( const fst::VectorFst<Arc>& myfst,
                                      unordered_set<std::string> *vcb ) {
  USER_CHECK ( vcb, "NULL pointer not accepted" );
  using fst::StateIterator;
  using fst::VectorFst;
  using fst::ArcIterator;
  typedef typename Arc::StateId StateId;
  for ( StateIterator< VectorFst<Arc> > si ( myfst ); !si.Done(); si.Next() ) {
    StateId state_id = si.Value();
    for ( ArcIterator< VectorFst<Arc> > ai ( myfst, si.Value() ); !ai.Done();
          ai.Next() ) {
      Arc arc = ai.Value();
      vcb->insert ( toString ( arc.ilabel ) );
    }
  }
};

/**
 * \brief Extract source (left-side) vocabulary (integers) from an fst.
 * \param myfst The fst or lattice for which we want to extract the vocabulary.
 * \param vcb To store the vocabulary (as Arc::Labels)
 * \param offset Offset summed to each label id
 */

template<class Arc >
inline void extractSourceVocabulary ( const fst::VectorFst<Arc>& myfst,
                                      unordered_set<unsigned> *vcb, 
                                      unsigned offset = 0) {
  USER_CHECK ( vcb, "NULL pointer not accepted" );
  using fst::StateIterator;
  using fst::VectorFst;
  using fst::ArcIterator;
  typedef typename Arc::StateId StateId;
  for ( StateIterator< VectorFst<Arc> > si ( myfst ); !si.Done(); si.Next() ) {
    StateId state_id = si.Value();
    for ( ArcIterator< VectorFst<Arc> > ai ( myfst, si.Value() ); !ai.Done();
          ai.Next() ) {
      vcb->insert ( ai.Value().ilabel + offset );
    }
  }
};

/**
 * \brief Extract target (right-side) vocabulary from an fst.
 * \param myfst: The fst or lattice for which we want to extract the vocabulary.
 * \param vcb: To store the vocabulary.
 */

template<class Arc>
void extractTargetVocabulary ( const fst::VectorFst<Arc>& myfst,
                               unordered_set<std::string> *vcb ) {
  USER_CHECK ( vcb, "NULL pointer not accepted" );
  typedef typename Arc::StateId StateId;
  using fst::StateIterator;
  using fst::VectorFst;
  using fst::ArcIterator;
  for ( StateIterator< VectorFst<Arc> > si ( myfst ); !si.Done(); si.Next() ) {
    StateId state_id = si.Value();
    for ( ArcIterator< VectorFst<Arc> > ai ( myfst, si.Value() ); !ai.Done();
          ai.Next() ) {
      Arc arc = ai.Value();
      vcb->insert ( ucam::util::toString ( arc.olabel ) );
    }
  }
};

/**
 * \brief Builds substring version of an fst. This is a destructive implementation.
 * \remark A substring fst accepts all the hypotheses in the original fst plus all its substrings.
 * A substring fst can be generated easily by setting all states to final and adding epsilon arcs from
 * start state to every state.
 * \param myfst: The input is the original fst. After running to completion, will contain the substring version of the fst.
 * \todo Improve this version taking out the if (state_id) condition.
 */

template<class Arc>
void buildSubstringTransducer ( fst::VectorFst<Arc> *myfst ) {
  USER_CHECK ( myfst, "NULL pointer not accepted" );
  USER_CHECK ( myfst->NumStates(), "Number of states is zero!" );
  typedef typename Arc::StateId StateId;
  fst::TopSort ( myfst );
  fst::Map ( myfst, fst::RmWeightMapper<Arc>() );
  for ( fst::StateIterator< fst::VectorFst<Arc> > si ( *myfst ); !si.Done();
        si.Next() ) {
    StateId state_id = si.Value();
    if ( state_id ) {
      myfst->AddArc ( 0, Arc ( 0, 0, Arc::Weight::One(), si.Value() ) );
      myfst->SetFinal ( state_id, Arc::Weight::One() );
    }
  }
  fst::RmEpsilon ( myfst );
};

/**
 * \brief Encodes, determinizes, minimizes and decodes an fst.
 * \remark This function is used approximate determinization to a transducer that is non-determinizable, i.e. the
 * transducer is not functional, as it can transduce more than one output string per single input string
 * (http://www.openfst.org/twiki/bin/view/FST/FstGlossary#FunctionalDef).
 * The transducer is encoded into an non-cyclic weighted automaton, which can be safely determinized and minimized.
 * Finally, the automaton is decoded back to a transducer.
 * Important: the function is destructive in the sense that myfst also gets modified for the encoding step.
 * \param myfst: The input is the original fst. <== Modified.
 * \returns pseudo-determinized fst.
 * \todo Make a const implementation or destructive implementation of this method.
 */

template <class Arc>
inline fst::VectorFst<Arc> *EncodeDeterminizeMinimizeDecode (
  fst::VectorFst<Arc> *myfst ) {
  fst::EncodeMapper<Arc> em ( fst::kEncodeLabels, fst::ENCODE );
  fst::Encode ( myfst, &em ); //note that this modifies fst1
  LDBG_EXECUTE ( myfst->Write ( "fsts/encoded.fst" ) );
  fst::VectorFst<Arc> *fst2 = new fst::VectorFst<Arc>;
  fst::Determinize ( *myfst, fst2 );
  fst::Minimize ( fst2 );
  fst::EncodeMapper<Arc> em2 ( em,
                               fst::DECODE ); //create em2 by copying em but now with decode option.
  fst::Decode ( fst2, em2 );
  return fst2;
};

template <class Arc>
void EncodeDeterminizeMinimizeDecode ( fst::Fst<Arc> const&  myfst ,
                                       fst::VectorFst<Arc> *out ) {
  out->DeleteStates();
  *out = (myfst);
  fst::EncodeMapper<Arc> em ( fst::kEncodeLabels, fst::ENCODE );
  fst::Encode ( out, &em );
  LDBG_EXECUTE ( out->Write ( "fsts/encoded.fst" ) );
  fst::VectorFst<Arc> fst2;
  fst::Determinize ( *out , &fst2 );
  fst::Minimize ( &fst2 );
  fst::EncodeMapper<Arc> em2 ( em,
                               fst::DECODE ); //create em2 by copying em but now with decode option.
  fst::Decode ( &fst2, em2 );
  out->DeleteStates();
  *out = (fst2);
};

/**
 * \brief Takes the 1-best of an fst and converts to string.
 * \param latfst Lattice from which we want to obtain the 1-best as a string.
 * \return basic_string, templated.
 * \remark Projects on the input first.
 */
template<class Arc,
         class CharTypeT,
				 class StringTypeT
				 >
inline std::basic_string<CharTypeT> 
FstGetBestHypothesis(const fst::VectorFst<Arc> &latfst) {
  using namespace fst;
	using namespace std;
  VectorFst<Arc> hypfst;
  ShortestPath(latfst, &hypfst);
  Project(&hypfst, PROJECT_INPUT);
  RmEpsilon(&hypfst);
  TopSort(&hypfst);
  basic_string<CharTypeT> hypstr;
  for (StateIterator< VectorFst<Arc> > si(hypfst); !si.Done();
       si.Next()) {
    for (ArcIterator< VectorFst<Arc> > ai(hypfst, si.Value());
         !ai.Done(); ai.Next()) {
      stringstream ss;
      ss << ai.Value().ilabel;
      StringTypeT value; ss >> value;
      hypstr += value; 
    }
  }
  return hypstr;
};


//basic_string to vector helper:
template<class Arc,
         class CharTypeT>
void FstGetBestHypothesis(const fst::VectorFst<Arc> &latfst
										 , std::vector<CharTypeT> &hyp) {	

	std::basic_string<CharTypeT> aux = FstGetBestHypothesis<Arc,CharTypeT, CharTypeT>(latfst);
	hyp.clear();
	hyp.resize(aux.size());
	std::copy(aux.begin(), aux.end(), hyp.begin());
}

//helper with std::string (spaces between numbers)
template<class Arc>
void FstGetBestStringHypothesis(const fst::VectorFst<Arc> &latfst
																, std::string &hyp) {
	std::basic_string<unsigned> aux =	FstGetBestHypothesis<Arc,unsigned, unsigned>(latfst);
	hyp.clear();
	for (unsigned k =0; k < aux.size(); ++k){
		std::stringstream ss; ss << aux[k];
		hyp += ss.str() + " ";
	}
}


/**
 * \brief Trivial function that outputs all the hypothesis in the lattice with its cost
 * \remarks. For big lattices, this method requires in practice no extra memory footprint than the FST itself,
 * unless hypotheses are incredibly large. The output is unsorted.
 * Hypotheses are printed with the following format:
 * lword1 lword2 ... lwordn || rword1 rword2 ... rwordm || weight[,weight]
 * Supports lexicographic semiring format, prints both weights.
 * \param pcostslat: Lattice we will take to print strings
 * \param hyps: Output stream (cout, a [file], ...)
 * \param s: Starting state.
 */

template<class Arc>
inline void printstrings ( const fst::VectorFst<Arc>& pcostslat,
                           std::ostream *hyps, unsigned s = 0 ) {
  static std::basic_string<unsigned> ihyp;
  static std::basic_string<unsigned> ohyp;
  static std::basic_string<typename Arc::Weight> cost;
  if ( !USER_CHECK ( s < pcostslat.NumStates(),
                     "The state most surely doesn't exist! Topsort this lattice and make sure you use a valid s" ) )
    return;
  typename Arc::Weight fvalue = Arc::Weight::One();
  fst::ArcIterator< fst::VectorFst<Arc> > i ( pcostslat, s );
  if ( !i.Done() ) {
    for ( ; !i.Done(); i.Next() ) {
      Arc a = i.Value();
      ihyp.push_back ( a.ilabel );
      ohyp.push_back ( a.olabel );
      cost.push_back ( a.weight );
      printstrings ( pcostslat, hyps, a.nextstate ); // recursive call to next state.
      ihyp.resize ( ihyp.size() - 1 );
      ohyp.resize ( ohyp.size() - 1 );
      cost.resize ( cost.size() - 1 );
    };
    fvalue = pcostslat.Final ( s );
    if ( fvalue == Arc::Weight::Zero() ) return;
  }
  for ( unsigned k = 0; k < ihyp.size();
        ++k ) if ( ihyp[k] != 0 ) *hyps  << ihyp[k] << " ";
  *hyps << "|| ";
  for ( unsigned k = 0; k < ihyp.size();
        ++k ) if ( ohyp[k] != 0 ) *hyps  << ohyp[k] << " ";
  *hyps << "|| ";
  typename Arc::Weight c = fvalue;
  for ( unsigned k = 0; k < cost.size(); ++k ) {
    c = Times ( c, cost[k] );
  }
  *hyps << c << endl;
};

///Struct for priority queue comparison
struct hypcost {
  std::string hyp;
  float cost;

};

///Class used by priority queue to compare two hypotheses and decide which one wins
class CompareHyp {
 public:
  bool operator() ( struct hypcost& h1, hypcost& h2 )   {
    if ( h1.cost > h2.cost ) return true;
    return false;
  }
};

///Prints input/output strings - weight from a lattice as a hash
template<class Arc>
inline void printstrings ( const fst::VectorFst<Arc>& fst
                           , unordered_map<std::string, float>& finalhyps
                           , bool input = true ) {
  fst::VectorFst<Arc> ofst ( fst );
  fst::TopSort ( &ofst ); //topological order is needed for the algorithm to work.
  std::vector <std::vector<struct hypcost> > partialhyps;
  partialhyps.clear();
  finalhyps.clear();
  struct hypcost hcempty;
  hcempty.cost = 0;
  std::vector<struct hypcost> vhcempty;
  partialhyps.resize ( ofst.NumStates() );
  partialhyps[0].push_back ( hcempty );
  for ( fst::StateIterator< fst::MutableFst<Arc> > si ( ofst ); !si.Done();
        si.Next() ) {
    typename Arc::Weight value = ofst.Final ( si.Value() );
    std::vector<struct hypcost>& hypc = partialhyps[ ( unsigned ) si.Value()];
    for ( unsigned k = 0; k < hypc.size(); ++k ) {
      if ( value != Arc::Weight::Zero() ) {
        finalhyps[partialhyps[ ( unsigned ) si.Value()][k].hyp] =
          partialhyps[ ( unsigned ) si.Value()][k].cost + ( float ) value.Value();
      }
      for ( fst::MutableArcIterator< fst::MutableFst<Arc> > ai ( &ofst, si.Value() );
            !ai.Done(); ai.Next() ) {
        struct hypcost hcnew = hypc[k];
        Arc arc = ai.Value();
        if ( input
             && arc.ilabel != 0 ) hcnew.hyp += " " + ucam::util::toString ( arc.ilabel );
        else if ( !input
                  && arc.olabel != 0 ) hcnew.hyp += " " + ucam::util::toString ( arc.olabel );
        hcnew.cost += ( float ) arc.weight.Value();
        partialhyps[arc.nextstate].push_back ( hcnew );
      }
    }
    partialhyps[ ( unsigned ) si.Value()] =
      vhcempty; //won't need these partial hyps any more
  }
};

inline unsigned ShortestPathLength (const fst::VectorFst<fst::StdArc>* fst) {
  using fst::StdArc;
  using fst::VectorFst;
  VectorFst<StdArc> tmp;
  fst::ShortestPath (*fst, &tmp);
  fst::RmEpsilon (&tmp);
  unsigned n = 0;
  for (fst::StateIterator< VectorFst<StdArc> > si (tmp); !si.Done(); si.Next() ) {
    for (fst::ArcIterator< VectorFst<StdArc> > ai (tmp, si.Value() ); !ai.Done();
         ai.Next() ) {
      n++;
    }
  }
  return n;
}

template <class Arc>
inline fst::VectorFst<Arc>* PushWeightsToFinal (const fst::VectorFst<Arc>*
    fst) {
  fst::VectorFst<Arc>* tmp = new fst::VectorFst<Arc>;
  fst::Push<Arc, fst::REWEIGHT_TO_FINAL> (*fst, tmp, fst::kPushWeights);
  return tmp;
}

inline fst::VectorFst<fst::LogArc>* StdToLog (const fst::VectorFst<fst::StdArc>*
    fst) {
  fst::VectorFst<fst::LogArc>* tmp = new fst::VectorFst<fst::LogArc>;
  fst::Map (*fst, tmp, fst::StdToLogMapper() );
  return tmp;
}

inline fst::VectorFst<fst::StdArc>* LogToStd (const fst::VectorFst<fst::LogArc>*
    fst) {
  using fst::StdArc;
  using fst::VectorFst;
  VectorFst<StdArc>* tmp = new VectorFst<StdArc>;
  fst::Map (*fst, tmp, fst::LogToStdMapper() );
  return tmp;
}

inline fst::VectorFst<fst::StdArc>* FstScaleWeights (fst::VectorFst<fst::StdArc>*
    fst
    , const double scale) {
  using fst::StdArc;
  using fst::VectorFst;
  VectorFst<StdArc>* fstscaled = fst->Copy();
  for (fst::StateIterator< VectorFst<StdArc> > si (*fstscaled); !si.Done();
       si.Next() ) {
    for (fst::MutableArcIterator< VectorFst<StdArc> > ai (fstscaled, si.Value() );
         !ai.Done(); ai.Next() ) {
      StdArc arc = ai.Value();
      arc.weight = static_cast<StdArc::Weight> (arc.weight.Value() * scale);
      ai.SetValue (arc);
    }
    StdArc::Weight final = fstscaled->Final (si.Value() );
    if (final != ZPosInfinity() ) {
      fstscaled->SetFinal (si.Value(),
                           static_cast<StdArc::Weight> (final.Value() * scale) );
    }
  }
  return fstscaled;
}

#define UNIT_COST_POSITIVE  1
#define UNIT_COST_NEGATIVE -1

template<class Arc>
inline void GetMinAndMaxHypothesisLength (const fst::VectorFst<Arc>* fst,
    unsigned& jMin, unsigned& jMax) {
  using fst::Map;
  using fst::RmWeightMapper;
  using fst::TimesMapper;
  fst::VectorFst<Arc>* tmp = fst->Copy();
  Map (tmp, RmWeightMapper<Arc, Arc>() );
  Map (tmp, TimesMapper<Arc> ( UNIT_COST_POSITIVE ) );
  jMin = ShortestPathLength (tmp);
  Map (tmp, RmWeightMapper<Arc, Arc>() );
  Map (tmp, TimesMapper<Arc> ( UNIT_COST_NEGATIVE ) );
  jMax = ShortestPathLength (tmp);
  delete tmp;
}

inline fst::StdArc::Label GetFirstUnusedLabelId (const
    fst::VectorFst<fst::StdArc>* fst) {
  using fst::StdArc;
  fst::StdArc::Label x = 0;
  for (fst::StateIterator< fst::VectorFst<StdArc> > si (*fst); !si.Done();
       si.Next() ) {
    for (fst::ArcIterator< fst::VectorFst<StdArc> > ai (*fst, si.Value() );
         !ai.Done(); ai.Next() ) {
      if (ai.Value().ilabel > x) {
        x = ai.Value().ilabel;
      }
    }
  }
  return x + 1;
}

inline void SetFinalStateCost (fst::MutableFst<fst::StdArc>* fst,
                               const fst::StdArc::Weight cost) {
  for (fst::StateIterator< fst::MutableFst<fst::StdArc> > si (*fst); !si.Done();
       si.Next() ) {
    if (fst->Final (si.Value() ) != ZPosInfinity() ) {
      fst->SetFinal (si.Value(), cost);
    }
  }
}

/**
 * \brief Convenience method that creates an fsa/fst from one/two string(s) of numbers.
 * \param sidxwords   : source sequence of words (numbers)
 * \param fst         : Output fst.
 * \param tidxwords   : target sequence of words (numbers)
 * \param finalweight : Weight of the fst.
 */
template<class Arc>
inline void string2fst (const std::string& sidxwords,
                        fst::VectorFst<Arc> *fst,
                        const std::string& tidxwords = "",
                        typename Arc::Weight finalweight = Arc::Weight::One()
                       ) {
  assert (sidxwords != "");
  std::vector<std::string> swords;
  boost::algorithm::split (swords, sidxwords,
                           boost::algorithm::is_any_of ( " " ) );
  fst->AddState();
  fst->SetStart (0);
  for (unsigned k = 0; k < swords.size(); ++k) {
    typename Arc::Label swidx = ucam::util::toNumber<unsigned> (swords[k]);
    fst->AddState();
    fst->AddArc (k, Arc (swidx, 0, Arc::Weight::One(), k + 1) );
  }
  if (tidxwords == "") {
    fst->SetFinal (swords.size(), Arc::Weight::One() );
    fst::Project<Arc> (fst,
                       fst::PROJECT_INPUT); //if only source provided, we assume an automaton.
    fst->SetFinal (swords.size(), finalweight);
    return;
  }
  std::vector<std::string> twords;
  boost::algorithm::split (twords, tidxwords,
                           boost::algorithm::is_any_of ( " " ) );
  for (unsigned k = swords.size(); k < swords.size() + twords.size(); ++k) {
    typename Arc::Label twidx = ucam::util::toNumber<unsigned>
                                (twords[k - swords.size()]);
    fst->AddState();
    fst->AddArc (k, Arc (0, twidx, Arc::Weight::One(), k + 1) );
  }
  fst->SetFinal (swords.size() + twords.size(), finalweight);
  fst::Determinize (fst::RmEpsilonFst<Arc> (*fst), fst);
  fst::Minimize (fst);
  fst::RmEpsilon (fst);
};

///Utility functor for relabeling one or more lattices. Note that you can chain commands. See Unit test in fstutils.gtest.cpp for an example.
template<class Arc>
class RelabelUtil {

 private:
  std::vector<pair <typename Arc::Label, typename Arc::Label> > ipairs;
  std::vector<pair <typename Arc::Label, typename Arc::Label> >  opairs;
 public:
  RelabelUtil() {};

  inline RelabelUtil& addIPL (typename Arc::Label labelfind,
                              typename Arc::Label labelreplace) {
    ipairs.push_back (pair <typename Arc::Label, typename Arc::Label> (labelfind,
                      labelreplace) );
    return *this;
  };
  inline RelabelUtil& addOPL (typename Arc::Label labelfind,
                              typename Arc::Label labelreplace) {
    opairs.push_back (pair <typename Arc::Label, typename Arc::Label> (labelfind,
                      labelreplace) );
    return *this;
  };
  inline RelabelUtil& operator() (fst::VectorFst<Arc> *hypfst) {
    fst::Relabel (hypfst, ipairs, opairs);
    return *this;
  }
  inline fst::VectorFst<Arc>& operator() (fst::VectorFst<Arc>& hypfst) {
    fst::Relabel (&hypfst, ipairs, opairs);
    return hypfst;
  }

};

} // end namespace

#endif
