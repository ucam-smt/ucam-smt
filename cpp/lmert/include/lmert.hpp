#ifndef LMERT_LMERT_HPP
#define LMERT_LMERT_HPP

#include <cmath>
#include <vector>
#include <functional>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>
#include <tr1/unordered_map>
#include <bleu.hpp>

namespace ucam {
namespace lmert {

typedef ucam::fsttools::Wid Wid;
typedef ucam::fsttools::Sid Sid;
typedef ucam::fsttools::SentenceIdx SentenceIdx;
typedef ucam::fsttools::PARAMS32 PARAMS32;

template <class Arc>
class MertLine {
 public:
  MertLine() : x ( -std::numeric_limits<double>::infinity() ), y ( 0.0 ),
    m ( 0.0 ) {}

  MertLine ( double y, double m, Wid word ) : 
    x ( -std::numeric_limits<double>::infinity() ), y ( y ), m ( m ) {}

  double x; // x-intercept of left-adjacent line
  double y; // y-intercept of line
  double m; // slope of line
  SentenceIdx t; // partial translation hypothesis associated with line
  double score; //translation score (quality)
  typename Arc::Weight weight;
};

template <class Arc>
class MertEnvelope {
 public:
	// lines that define the envelope / convex hull
  std::vector<MertLine<Arc> > lines; 

  static bool GradientSortPredicate ( const MertLine<Arc>& line1,
                                      const MertLine<Arc>& line2 ) {
    return line1.m < line2.m;
  }

  MertEnvelope() {};

  // sort envelope lines by slope
  void SortLines() {
    sort ( lines.begin(), lines.end(), GradientSortPredicate );
  }

  // compute upper envelope of lines in array
  void SweepLine() {
    SortLines();
    int j = 0;

    for ( typename std::vector<MertLine<Arc> >::size_type i = 0; i < lines.size(); i++ ) {
      MertLine<Arc> l = lines[i];
      l.x = -std::numeric_limits<double>::infinity();
      if ( 0 < j ) {
        if ( lines[j - 1].m == l.m ) {
          if ( l.y <= lines[j - 1].y )
            continue;
          --j;
        }
        while ( 0 < j ) {
          l.x = ( l.y - lines[j - 1].y ) / ( lines[j - 1].m - l.m );

          if ( lines[j - 1].x < l.x )
            break;

          --j;
        }
        if ( 0 == j )
          l.x = -std::numeric_limits<double>::infinity();

        lines[j++] = l;
      } else {
        lines[j++] = l;
      }
    }
    lines.resize ( j );
  }

  // returns lines that constitute the envelope as a string
  std::string ToString ( bool show_hypothesis = false ) {
    std::ostringstream oss;

    for ( typename std::vector<MertLine<Arc> >::size_type i = 0; i < lines.size();
          ++i ) {
      oss << "line i=[" << std::right << std::setw ( 4 ) << i << "]" << std::fixed
          << std::setprecision ( 6 ) << " x=[" << std::right << std::setw (
            12 ) << lines[i].x
          << "]" << " y=[" << std::right << std::setw ( 12 ) << lines[i].y << "]"
          << " m=[" << std::right << std::setw ( 12 ) << lines[i].m << "]";

      if ( show_hypothesis ) {
        oss << " t=[" << lines[i].t << "]";
      }

      oss << " w=[" << lines[i].weight << "]";
      oss << std::endl;
    }

    return oss.str();
  }

};

template <class Arc>
class MertLattice {
 public:
  MertEnvelope<Arc> finalEnvelope;
  std::vector<ucam::fsttools::BleuStats> prev;

  MertLattice (fst::VectorFst<Arc>* fst, const PARAMS32& lambda, const PARAMS32& direction ) :
    fst_ ( fst ), lambda_ ( lambda ), direction_ ( direction ) {
    finalEnvelope.lines.clear();
    // InializeEnvelopes()
    envelopes_.clear();
    envelopes_.resize ( fst_->NumStates() + 1 );
    // InitializeStartState()
    envelopes_[fst->Start()].lines.push_back ( MertLine<Arc>() );
    // ComputeStateEnvelopes()
    for ( fst::StateIterator < fst::VectorFst<Arc> > si ( *fst ); !si.Done();
          si.Next() ) {
      const typename Arc::StateId& s = si.Value();
      envelopes_[s].SweepLine();
      for ( fst::ArcIterator < fst::VectorFst<Arc> > ai ( *fst, si.Value() );
            !ai.Done(); ai.Next() ) {
        const Arc& a = ai.Value();
        PropagateEnvelope ( s, a.nextstate, a.weight, a.ilabel );
      }
      if ( fst->Final ( s ) != Arc::Weight::Zero() ) {
        PropagateEnvelope ( s, fst->NumStates(), fst->Final ( s ) );
      }
      envelopes_[s].lines.clear();
    }
    // ComputeFinalEnvelopes()
    envelopes_[fst->NumStates()].SweepLine();
    //
    finalEnvelope = envelopes_[fst->NumStates()];
    envelopes_[fst->NumStates()].lines.clear();
  }

  void PropagateEnvelope ( const typename Arc::StateId& src,
                           const typename Arc::StateId& trg, const typename Arc::Weight& weight,
                           const Wid& w = 0 ) {
    for ( unsigned int i = 0; i < envelopes_[src].lines.size(); ++i ) {
      MertLine<Arc> line ( envelopes_[src].lines[i] );
      line.y += fst::DotProduct<float> ( weight, lambda_ ) * -1;
      line.m += fst::DotProduct<float> ( weight, direction_ ) * -1;
      line.weight = fst::Times<float> ( weight, line.weight );
      if ( w != 0 ) {
        line.t.push_back ( w );
      }
      envelopes_[trg].lines.push_back ( line );
    }
  }

 private:
  fst::VectorFst<Arc>* fst_;
  std::vector<MertEnvelope<Arc> > envelopes_;
  PARAMS32 lambda_;
  PARAMS32 direction_;
};


template <class Arc>
class MertLatticeWrap {
 public:
  MertLatticeWrap ( Sid sidx, fst::VectorFst<Arc>* fst, const PARAMS32& lambda,
                    const PARAMS32& direction, vector< MertEnvelope<Arc> >& env ) :
    sid_ ( sidx ), fst_ ( fst ), lambda_ ( lambda ), direction_ ( direction ),
    env_ ( env ) {}

  void operator() () {
    MertLattice<Arc> ml ( fst_, lambda_, direction_ );
    env_[sid_] = ml.finalEnvelope;
  };

  Sid sid_;
  fst::VectorFst<Arc>* fst_;
  PARAMS32 lambda_;
  PARAMS32 direction_;
  vector< MertEnvelope<Arc> >& env_;
};

}}  // end namespaces
#endif
