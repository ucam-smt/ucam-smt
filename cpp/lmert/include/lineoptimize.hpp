#ifndef LINEOPTIMIZE_HPP
#define LINEOPTIMIZE_HPP

#include <multithreading.hpp>

class IntervalBoundary {
public:
  IntervalBoundary(){};

  IntervalBoundary(Sid sentence,  const double gamma, BleuStats bleuStats): sentence_(sentence), gamma_(gamma), bleuStats_(bleuStats){}

  Sid sentence_;
  BleuStats bleuStats_;
  double gamma_;
};

std::ostream& operator<< (std::ostream&os, const IntervalBoundary& b) {
  os << b.sentence_ << "; gamma:  " << b.gamma_ << "; bleuStats: " << b.bleuStats_;
}

template<typename IntervalBoundary>
bool IntervalBoundarySortPredicate (const IntervalBoundary& b1, const IntervalBoundary& b2) {
  return b1.gamma_ < b2.gamma_;
}

template <class Arc>
class LineOptimize {
public:
  LineOptimize(ucam::util::RegistryPO const& rg, TuneSet<Arc> const& ts, BleuScorer& bs, PARAMS32 lambda, PARAMS32 direction): lambda_(lambda), direction_(direction), nthreads_(rg.get<int>("num_threads")){
    envelopes_.clear();
    envelopes_.resize(ts.sidMax);    
    {
#ifdef NO_MULTI_THREADING
      for (Sid sidx=0; sidx<ts.sidMax; sidx++) {
	MertLattice<Arc> env(sidx, ts.cachedLats[sidx], lambda_, direction_);
	envelopes_[sidx] = env.finalEnvelope;
      }
#else
      ucam::util::TrivialThreadPool tp(nthreads_);
      for (Sid sidx=0; sidx<ts.sidMax; sidx++) {
	MertLatticeWrap<Arc> envw(sidx, ts.cachedLats[sidx], lambda_, direction_, envelopes_);
	tp ( envw );
      }
#endif
    }
    prev.resize( envelopes_.size() );
    initials.resize( envelopes_.size() );
    for (Sid sidx=0; sidx<envelopes_.size(); sidx++) {
      MertEnvelope<Arc> env = envelopes_[sidx];
      // iterate over lines
      typename std::vector<MertLine<Arc> >::size_type i = 0;
      // Trim sentence start and end markers from hypothesis
      int offset = (env.lines[i].t.size() < 2 ? 0 : 1);
      SentenceIdx h (env.lines[i].t.begin() + offset, env.lines[i].t.end() - offset); 
      // CreateInitial()
      prev[sidx] = bs.SentenceBleuStats(sidx, h);
      IntervalBoundary bd1(sidx, env.lines[i].x, prev[sidx]);
      initials[sidx] = bd1;
      // CreateInterval()
      for (i=1; i<env.lines.size(); ++i) {    
	offset = (env.lines[i].t.size() < 2 ? 0 : 1);
	h.assign(env.lines[i].t.begin() + offset, env.lines[i].t.end() - offset); 
	BleuStats next = bs.SentenceBleuStats(sidx, h);
	IntervalBoundary bd(sidx, env.lines[i].x, next-prev[sidx]);
	//	std::cerr << bd << std::endl;
	boundaries.push_back(bd);
	prev[sidx] = next;
      }      
    }
    Surface(bs);
  }

  // Compute Surface()
  void Surface(BleuScorer& bs) {
    std::vector<IntervalBoundary> currentIBs (initials);
    BleuStats aggregateBleuStats;
    // MergeInitialScores() and MergeInitials()
    for (typename std::vector<IntervalBoundary>::const_iterator it = initials.begin(); it != initials.end(); ++it) {
      aggregateBleuStats = aggregateBleuStats + it->bleuStats_;
    }    
    //    std::cerr << aggregateBleuStats << std::endl;
    optimalGamma = -std::numeric_limits<double>::infinity();
    if (boundaries.size() == 0) {
      std::cerr << "no boundaries - returning" << std::endl;
      return;
    }
    sort (boundaries.begin(), boundaries.end(), IntervalBoundarySortPredicate<IntervalBoundary>);
    unbounded = true;     // initial interval is unbounded
    optimalGamma = boundaries.front().gamma_ - 1;
    optimalBleu = bs.ComputeBleu( aggregateBleuStats );
    //    std::cerr << "OO " << optimalGamma << " " << optimalBleu << " :: " << boundaries.size() << std::endl;
    typename std::vector<IntervalBoundary>::iterator itNext = ++ (boundaries.begin() );
    for (typename std::vector<IntervalBoundary>::iterator it = boundaries.begin(); it != boundaries.end(); ++it) {
      aggregateBleuStats = aggregateBleuStats + it->bleuStats_;
      Bleu current = bs.ComputeBleu( aggregateBleuStats );
      if (current > optimalBleu) {
        optimalBleu = current;
	unbounded = (itNext == boundaries.end());
	optimalGamma = (itNext == boundaries.end()) ? it->gamma_ + 1.0 : 0.5 * (it->gamma_ + itNext->gamma_);
      }
      ++itNext;
    }    
  }

  double OptimalGamma() {
    return optimalGamma;
  }

  Bleu OptimalBleu() {
    return optimalBleu;
  }

private:
  int nthreads_;
  PARAMS32 lambda_;
  PARAMS32 direction_;
  vector< MertEnvelope<Arc> > envelopes_;
  std::vector<BleuStats> prev;
  std::vector<IntervalBoundary> initials;
  std::vector<IntervalBoundary> boundaries;
  double optimalGamma;
  Bleu optimalBleu;
  bool unbounded;
};


#endif
