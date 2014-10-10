#ifndef LMERT_LINEOPTIMIZE_HPP
#define LMERT_LINEOPTIMIZE_HPP

#include <multithreading.helpers.hpp>
#include <bleu.hpp>
#include <tuneset.hpp>

namespace ucam {
namespace lmert {

class IntervalBoundary {
public:
  IntervalBoundary() {};

  IntervalBoundary ( Sid sentence,  const double gamma,
		     ucam::fsttools::BleuStats const &bleuStats ) :
    sentence_ ( sentence ),
    gamma_ ( gamma ),
    bleuStats_ ( bleuStats ) {}

  Sid sentence_;
  ucam::fsttools::BleuStats bleuStats_;
  double gamma_;
};

std::ostream& operator<< ( std::ostream&os, const IntervalBoundary& b ) {
  os << b.sentence_ << "; gamma:  " << b.gamma_ << "; bleuStats: " <<
    b.bleuStats_;
}

template<typename IntervalBoundary>
bool IntervalBoundarySortPredicate ( const IntervalBoundary& b1
                                     , const IntervalBoundary& b2 ) {
  return b1.gamma_ < b2.gamma_;
}


template <class Arc>
class LineOptimize {
public:
  LineOptimize ( ucam::util::RegistryPO const& rg,
                 ucam::fsttools::TuneSet<Arc> const& ts,
                 ucam::fsttools::BleuScorer& bs,
                 PARAMS32 const &lambda,
                 PARAMS32 const &direction ) :
    lambda_ ( lambda ),
    direction_ ( direction ),
    nthreads_ ( rg.get<int> ( HifstConstants::kNThreads.c_str()) ) {
    envelopes_.clear();
    envelopes_.resize ( ts.sidMax );
    {
#ifdef NO_MULTI_THREADING

      for ( Sid sidx = 0; sidx < ts.sidMax; sidx++ ) {
        MertLattice<Arc> env ( sidx, ts.cachedLats[sidx], lambda_, direction_ );
        envelopes_[sidx] = env.finalEnvelope;
      }

#else
      ucam::util::TrivialThreadPool tp ( nthreads_ );

      for ( Sid sidx = 0; sidx < ts.sidMax; sidx++ ) {
        MertLatticeWrap<Arc> envw ( sidx, &*(ts.cachedLats[sidx]), lambda_, direction_,
                                    envelopes_ );
        tp ( envw );
      }

#endif
    }
    prev.resize ( envelopes_.size() );
    initials.resize ( envelopes_.size() );
    for ( Sid sidx = 0; sidx < envelopes_.size(); sidx++ ) {
      MertEnvelope<Arc> env = envelopes_[sidx];
      // iterate over lines
      typename std::vector<MertLine<Arc> >::size_type i = 0;
      // Trim sentence start and end markers from hypothesis
      int offset = ( env.lines[i].t.size() < 2 ? 0 : 1 );
      SentenceIdx h ( env.lines[i].t.begin() + offset,
                      env.lines[i].t.end() - offset );
      // CreateInitial()
      prev[sidx] = bs.SentenceBleuStats ( sidx, h );
      IntervalBoundary bd1 ( sidx, env.lines[i].x, prev[sidx] );
      initials[sidx] = bd1;
      // CreateInterval()
      for ( i = 1; i < env.lines.size(); ++i ) {
        offset = ( env.lines[i].t.size() < 2 ? 0 : 1 );
        h.assign ( env.lines[i].t.begin() + offset, env.lines[i].t.end() - offset );
	ucam::fsttools::BleuStats next = bs.SentenceBleuStats ( sidx, h );
        IntervalBoundary bd ( sidx, env.lines[i].x, next - prev[sidx] );
        //	std::cerr << bd << std::endl;
        boundaries.push_back ( bd );
        prev[sidx] = next;
      }
    }
    Surface ( bs );
  }

  // Compute Surface()
  void Surface ( ucam::fsttools::BleuScorer& bs ) {
    std::vector<IntervalBoundary> currentIBs ( initials );
    ucam::fsttools::BleuStats aggregateBleuStats;

    // MergeInitialScores() and MergeInitials()
    for ( typename std::vector<IntervalBoundary>::const_iterator it =
            initials.begin(); it != initials.end(); ++it ) {
      aggregateBleuStats = aggregateBleuStats + it->bleuStats_;
    }
    optimalGamma = -std::numeric_limits<double>::infinity();
    if ( boundaries.size() == 0 ) {
      LINFO("no boundaries - returning");
      return;
    }
    sort ( boundaries.begin(), boundaries.end(),
           IntervalBoundarySortPredicate<IntervalBoundary> );
    unbounded = true;     // initial interval is unbounded
    optimalGamma = boundaries.front().gamma_ - 1;
    optimalBleu = bs.ComputeBleu ( aggregateBleuStats );
    //    std::cerr << "OO " << optimalGamma << " " << optimalBleu << " :: " << boundaries.size() << std::endl;
    typename std::vector<IntervalBoundary>::iterator itNext = ++( boundaries.begin() );
    for ( typename std::vector<IntervalBoundary>::iterator it = boundaries.begin();
          it != boundaries.end(); ++it ) {
      aggregateBleuStats = aggregateBleuStats + it->bleuStats_;
      ucam::fsttools::Bleu current = bs.ComputeBleu ( aggregateBleuStats );
      if ( current > optimalBleu ) {
        optimalBleu = current;
        unbounded = ( itNext == boundaries.end() );
        optimalGamma = ( itNext == boundaries.end() ) ? it->gamma_ + 1.0 : 
	  0.5 * ( it->gamma_ + itNext->gamma_ );
      }
      ++itNext;
    }
  }

  double OptimalGamma() {
    return optimalGamma;
  }

  ucam::fsttools::Bleu OptimalBleu() {
    return optimalBleu;
  }

private:
  int nthreads_;
  PARAMS32 lambda_;
  PARAMS32 direction_;
  std::vector< MertEnvelope<Arc> > envelopes_;
  std::vector<ucam::fsttools::BleuStats> prev;
  std::vector<IntervalBoundary> initials;
  std::vector<IntervalBoundary> boundaries;
  double optimalGamma;
  ucam::fsttools::Bleu optimalBleu;
  bool unbounded;
};

}}  // end namespaces
#endif
