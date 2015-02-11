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

#ifndef TASK_DISAMBIG_FLOWER_HPP
#define TASK_DISAMBIG_FLOWER_HPP

/**
 * \file
 * \brief Utilities for DisambigTask and related tasks
 * \date October 2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

// Applies scale factor to grammar at each cost in the arc.
template <class Arc>
void SetGsf ( fst::VectorFst<Arc> *grmfst, const float gsf ) {
  if ( gsf == 1.0f )  return;
  fst::ScaleWeight<Arc> sw ( gsf );
  fst::Map<Arc> ( grmfst,
                  fst::GenericWeightAutoMapper<Arc, fst::ScaleWeight<Arc> > ( sw ) );
};

template <>
void SetGsf ( fst::VectorFst<TupleArc32> *grmfst, const float gsf ) {
  LERROR("Sorry, TupleArc32 not supported!");
  exit(EXIT_FAILURE);
};


///Loads flower fst from srilm disambig unigram input file
template < class Arc >
inline void loadflowerfst ( ucam::util::iszfstream& umf,
                            fst::VectorFst<Arc>& flowerlattice ) {
  std::string line;
  typedef typename Arc::Weight Weight;
  flowerlattice.AddState();
  flowerlattice.SetStart ( 0 );
  flowerlattice.SetFinal ( 0, Weight::One() );
  fst::MakeWeight<Arc> mw;
  while ( umf.getline ( line ) ) {
    using ucam::util::toNumber;
    std::vector<std::string> aux;
    boost::algorithm::split ( aux, line, boost::algorithm::is_any_of ( " " ) );
    USER_CHECK ( aux.size() % 2, "Wrong unimap input file format" );
    if ( aux[0] == "<unk>" ) continue; //skip this line
    if ( aux[0] == "<s>" ) aux[0] = "1";
    if ( aux[1] == "<s>" ) aux[1] = "1";
    if ( aux[0] == "</s>" ) aux[0] = "2";
    if ( aux[1] == "</s>" ) aux[1] = "2";
    for ( unsigned k = 1; k < aux.size(); k += 2 ) {
      float w = -std::log ( toNumber<float> ( aux[k + 1] ) ) ;
      if (w != std::numeric_limits<float>::infinity() )
        flowerlattice.AddArc ( 0, Arc ( toNumber<unsigned> ( aux[0] ),
                                        toNumber<unsigned> ( aux[k] ), mw ( w ), 0 ) );
      else
        LWARN ("Skipping 0 probability at line:" << line);
    }
  }
  flowerlattice.AddArc ( 0, Arc ( RHO, RHO, Weight::One(), 0 ) );
  flowerlattice.AddArc ( 0, Arc ( OOV, OOV, Weight::One(), 0 ) ); //for OOVs...
  ArcSort ( &flowerlattice, fst::ILabelCompare<Arc>() );
};

/// Identifies OOVs in word lattice
/// and rewrites them using a special tag on output side (OOV)
template<class Arc>
inline void tagOOVs ( fst::VectorFst<Arc> *myfst,
                      unordered_set<std::string>& vcb ) {
  fst::MakeWeight<Arc> mw;
  typedef typename Arc::StateId StateId;
  for ( fst::StateIterator< fst::VectorFst<Arc> > si ( *myfst ); !si.Done();
        si.Next() ) {
    StateId state_id = si.Value();
    for ( fst::MutableArcIterator< fst::MutableFst<Arc> > ai ( myfst, si.Value() );
          !ai.Done(); ai.Next() ) {
      Arc arc = ai.Value();
      if ( vcb.find ( ucam::util::toString<unsigned> ( arc.olabel ) ) == vcb.end() ) {
        arc.ilabel = arc.olabel;
        arc.olabel = OOV;
        arc.weight = mw ( 0 );
        ai.SetValue ( arc );
      }
    }
  }
};

///Recover OOV original ids by projecting selectively

template<class Arc>
inline void recoverOOVs ( fst::VectorFst<Arc> *myfst ) {
  typedef typename Arc::StateId StateId;
  for ( fst::StateIterator< fst::VectorFst<Arc> > si ( *myfst ); !si.Done();
        si.Next() ) {
    StateId state_id = si.Value();
    for ( fst::MutableArcIterator< fst::MutableFst<Arc> > ai ( myfst, si.Value() );
          !ai.Done(); ai.Next() ) {
      Arc arc = ai.Value();
      if ( arc.olabel == OOV ) {
        arc.olabel = arc.ilabel;
        ai.SetValue ( arc );
      }
    }
  }
};

}
} // end namespaces

#endif // FSTDISAMBIG_HPP

