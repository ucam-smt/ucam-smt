//Copyright (c) 2012, University of Cambridge
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met://
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of Cambridge nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef OPTIMIZE_H_
#define OPTIMIZE_H_

#include "TuneSet.h"
#include "ParamsConfig.h"
#include "Score.h"
#include <utility>
#include <BleuStats.h>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <global_incls.hpp>
#include <main.custom_assert.hpp>
#include <multithreading.hpp>

///hifst-specific classes and methods included in this namespace.

struct MERT {
  ParamsConfig limits; // Maximum and minimum values for delta gamma
};

extern MERT mert;

class Optimizer {
 public:
  virtual const pair<PARAMS, double> operator() (PARAMS&) = 0;

  virtual void Init (int) = 0;

  virtual std::string ComputeError (const PARAMS& lambda) = 0;

  virtual void LoadRefData (std::vector<std::string>) = 0;

  virtual void InitTuneSet (bool) = 0;

  virtual ~Optimizer() {
  }
  ;

};

class RandDirGenerator {

  unsigned int dim;
  boost::random::mt19937 rand_gen;

 public:

  RandDirGenerator (unsigned int dim = 0);

  PARAMS GenerateDirection();

};

template<typename Algo, typename ErrorSurface>
class OptimizeTask {

  Sid sid;
  PARAMS const *lambda;
  PARAMS const *direction;
  TuneSet const *lats;
  ErrorSurface *surface;

 public:

  OptimizeTask (const Sid sid, PARAMS const* lambda, PARAMS const* direction,
                TuneSet const * lats, ErrorSurface *surface) :
    sid (sid), lambda (lambda), direction (direction), lats (lats), surface (
      surface) {
  }

  void operator() () {
    if (opts.verbose) {
      tracer << "lattice line optimization: sentence s=" << sid << '\n';
    }
    TupleArcFst* fst = lats->GetVectorLattice (sid, opts.useCache);
    if (!fst) {
      std::cerr << "ERROR: invalid vector lattice for sentence s=" << sid
                << '\n';
      exit (1);
    }
    //PruneStats pruneStats;
    const typename Algo::Lines lines = Algo::ComputeLatticeEnvelope (fst,
                                       *lambda, *direction);
    /*
     if (opts.verbose) {
     tracer << "Arcs in lattices before prune: " << pruneStats.before
     << endl;
     tracer << "Arcs in lattice after prune: " << pruneStats.after
     << endl;
     }*/
    double prevScore = 0.0;
    double prevExpScore = 0.0;
    for (typename Algo::Lines::const_iterator line = lines.begin();
         line != lines.end(); ++line) {
      //Sometimes we get empty hypotheses
      int offset;
      line->t.size() < 2 ? offset = 0 : offset = 1;
      Sentence h (line->t.begin() + offset,
                  line->t.end() - offset); // Trim sentence start and end markers from hypothesis
      double expScore;
      if (line == lines.begin() ) {
        expScore = ( (++lines.begin() )->x - 1) * line->m + line->y;
        surface->CreateInitial (sid, line->x, h, line->score, expScore);
      } else {
        expScore = line->x * line->m + line->y;
        surface->CreateInterval (sid, line->x, h, line->score - prevScore,
                                 expScore - prevExpScore);
      }
      prevScore = line->score;
      prevExpScore = expScore;
    }
    if (!opts.useCache) {
      delete fst;
    }
  }

};

int GetNoOfThreads();

template<class Algo, class ErrorSurface>
void execute_with_threadpool (std::vector<OptimizeTask<Algo, ErrorSurface> >
                              tasks) {
  ucam::util::TrivialThreadPool tp (GetNoOfThreads() );
  for (typename std::vector<OptimizeTask<Algo, ErrorSurface> >::const_iterator it
       =
         tasks.begin(); it != tasks.end(); ++it) {
    tp (*it);
  }
}

PARAMS ComputeFinalPoint (const PARAMS&, const PARAMS&, const double);

template<class Algo, class ErrorSurface>
class OptimizerImpl: public Optimizer {

 protected:
  typedef typename ErrorSurface::ErrorStats ErrorStats;
  typedef typename ErrorStats::Error Error;
  typedef pair<PARAMS, Error> OptimizationResult;

  OptimizationResult MakeOptimizationResult (ErrorSurface& surface,
      const PARAMS& direction, const OptimizationResult& prev) {
    if (surface.boundaries.empty() ) {
      return prev;
    }
    surface.ComputeSurface();
    PARAMS tunedPoint = ComputeFinalPoint (prev.first, direction,
                                           surface.GetOptimalGamma() );
    return make_pair (tunedPoint, surface.GetOptimalError() );
  }

  void LineOptimize (const PARAMS& lambda, const PARAMS& direction,
                     ErrorSurface& surface, const std::vector<Sid>& lattices) {
    std::vector<OptimizeTask<Algo, ErrorSurface> > tasks;
    for (std::vector<Sid>::const_iterator sit = lattices.begin();
         sit != lattices.end(); ++sit) {
      OptimizeTask<Algo, ErrorSurface> task (*sit, &lambda, &direction,
                                             & (this->lats), &surface);
      tasks.push_back (task);
    }
    execute_with_threadpool (tasks);
  }

  unsigned int GetBestOptimizationDirection (
    const std::vector<OptimizationResult>& results) {
    double max = 0.0;
    unsigned int d = 0;
    for (unsigned int k = 0; k < results.size(); ++k) {
      double bleu = results[k].get_tunedScore().GetError()
                    - results[k].get_startScore().GetError();
      if (bleu > max) {
        max = bleu;
        d = k;
      }
    }
    return d;
  }

  void LogLineOptimization (const OptimizationResult& start,
                            const OptimizationResult& tuned, const PARAMS& direction,
                            const double gamma) {
    Error startError = start.second;
    Error tunedError = tuned.second;
    if (opts.fullLog) {
      const PARAMS& startPoint = start.first;
      const PARAMS& tunedPoint = tuned.first;
      tracer << "start: " << std::fixed << std::setprecision (opts.printPrecision)
             << startPoint << " " << std::setprecision (6) << startError
             << '\n';
      tracer << "  direction: " << std::fixed
             << std::setprecision (opts.printPrecision) << direction << '\n';
      tracer << "  final: " << std::fixed << std::setprecision (opts.printPrecision)
             << tunedPoint << std::setprecision (6) << " " << tunedError
             << '\n';
      tracer << "  gamma: " << std::fixed << std::setprecision (8) << gamma << '\n';
    } else {
      // Print every hundreth gamma
      static unsigned int counter = 0;
      static const unsigned int dim = direction.size();
      static const unsigned int percent = dim < 1000 ? 1 : dim / 1000;
      counter = (counter + 1) % dim;
      if (counter % percent == 0) {
        tracer << "gamma: " << gamma << " error: " << tunedError
               << endl;
      }
    }
  }

  typename ErrorSurface::RefData refData;
  TuneSet lats; // Vector lattices for each sentence

 public:

  virtual void LoadRefData (std::vector<std::string> refs) {
    refData.LoadRefData (refs);
  }

  virtual void InitTuneSet (bool useCache) {
    lats.Initialize (useCache);
  }

  virtual ~OptimizerImpl() {
  }

  virtual std::string ComputeError (const PARAMS& lambda) {
    return Score (this->refData, this->lats, lambda);
  }
};

/*
 * Scale a PARAMS by the a single parameter value. Designed to combat numerical instability.
 */

PARAMS VectorScale (const PARAMS&, const unsigned int k = 0);

PARAMS GenerateRandDir (const unsigned int noOfAxes);

class Directions {

 public:

  Directions();

  const PARAMS& Get (const unsigned int);

  void Set (const unsigned int, PARAMS);

  void Set (std::vector<PARAMS>, TuneSet&);

  unsigned int Size();

  void Resize (unsigned int, TuneSet& lats);

  const std::vector<Sid> FilteredLattices (unsigned int, const std::vector<Sid>&);

  bool ContainsAxis (unsigned int);

 private:

  bool ContainsAxis (Sid, unsigned int);

  unordered_map<unsigned int, PARAMS> directions;

  unsigned int dim;

  unsigned int prev;

  PARAMS current;

  // Stores the features that are populated for each axis
  unordered_map<Sid, unordered_set<unsigned int> > feature_filter;

  bool * hasDirection;

};

template<typename Algo, typename ErrorSurface>
class RandomOptimizer: public OptimizerImpl<Algo, ErrorSurface> {

  typedef typename OptimizerImpl<Algo, ErrorSurface>::OptimizationResult
  OptimizationResult;
  typedef typename OptimizerImpl<Algo, ErrorSurface>::Error Error;
  RandDirGenerator dirGen;
  unsigned int dim;
  unsigned int directions;
  bool useAxes;

 public:

  virtual void Init (int);

  virtual ~RandomOptimizer() {}

  virtual const pair<PARAMS, double> operator() (PARAMS& startPoint) {
    Error startError = ComputeError (this->refData, this->lats, startPoint);
    OptimizationResult start = make_pair (startPoint, startError);
    OptimizationResult prev = start;
    while (true) {
      OptimizationResult best = prev;
      std::vector<PARAMS> generatedDirs;
      if (useAxes) {
        for (int i = 0; i < dim; ++i) {
          PARAMS dir = PARAMS (dim);
          for (int j = 0; j < dim; ++j) {
            dir[j] = 0;
          }
          dir[i] = 1;
          generatedDirs.push_back (dir);
        }
      }
      for (int i = 0; i < directions; ++i) {
        generatedDirs.push_back (dirGen.GenerateDirection() );
      }
      for (std::vector<PARAMS>::const_iterator direction =
             generatedDirs.begin(); direction != generatedDirs.end();
           ++direction) {
        ErrorSurface surface (this->lats.ids.size(), & (this->refData) );
        this->LineOptimize (prev.first, *direction, surface, this->lats.ids);
        OptimizationResult current = this->MakeOptimizationResult (surface,
                                     *direction, prev);
        this->LogLineOptimization (prev, current, *direction,
                                   surface.GetOptimalGamma() );
        if (surface.boundaries.size() == 0) {
          if (opts.fullLog) {
            tracer <<
                   "    parameters not updated: no interval boundaries\n";
          }
          continue;
        }
        //Need to do limits check properly
        //if (!mert.limits.check_in_range(i, surface.GetOptimalGamma())) {
        //  continue;
        //}
        if (abs (surface.GetOptimalGamma() ) < opts.gammaThreshold) {
          if (opts.fullLog) {
            tracer <<
                   "    parameters not updated: change in gamma < "
                   << opts.gammaThreshold << '\n';
          }
          continue;
        }
        if (current.second > best.second) {
          best = current;
        }
      }
      if (!opts.fullLog) {
        tracer << "full random iteration completed with error: "
               << best.second << '\n';
        std::cerr << std::fixed << std::setprecision (opts.printPrecision)
                  << VectorScale (best.first) << endl;
      }
      if (best.second.GetError() - prev.second.GetError()
          < opts.bleuThreshold) {
        tracer <<
               "no direction gives sufficient improvement: delta bleu < "
               << opts.bleuThreshold << '\n';
        break;
      }
      prev = best;
    }
    return make_pair (prev.first, prev.second.GetError() );
  }
};

template<typename Algo, typename ErrorSurface>
class PowellOptimizer: public OptimizerImpl<Algo, ErrorSurface> {
 public:
  typedef typename OptimizerImpl<Algo, ErrorSurface>::OptimizationResult
  OptimizationResult;
  typedef typename OptimizerImpl<Algo, ErrorSurface>::Error Error;
 private:
  Directions directions;

  void UpdateDirection (const OptimizationResult& start,
                        const OptimizationResult& final, double bestChange,
                        unsigned int bestDirection, Directions& directions) {
    PARAMS extrapolated_d = final.first -
                            start.first; // Compute new extrapolated direction
    PARAMS extrapolated_p = final.first +
                            extrapolated_d; // Compute new extrapolated point
    double dbm = bestChange; // Delta bleu max
    double dbt = final.second.GetError() -
                 start.second.GetError(); // Delta bleu total
    double dbe =
      ComputeError (this->refData, this->lats, extrapolated_p).GetError()
      - start.second.GetError(); // Delta bleu extrapolated
    if (dbe > 0
        && 2 * (2 * dbt - dbe) * powf (dbt - dbm, 2.0)
        < powf (dbe, 2.0) * dbm) { // Numerical Recipes in C++ (Second Edition) (p422)
      directions.Set (bestDirection, VectorScale (extrapolated_d) );
      tracer << "replacing direction[" << bestDirection << "]" << std::fixed
             << std::setprecision (opts.printPrecision) << '\n';
    }
  }

 public:

  virtual void Init (int);

  virtual const pair<PARAMS, double> operator() (PARAMS& startPoint) {
    ErrorSurface surface (this->lats.ids.size(), & (this->refData) );
    Error startError = ComputeError (this->refData, this->lats, startPoint);
    OptimizationResult start = make_pair (startPoint, startError);
    OptimizationResult prev;
    while (true) {
      prev = start;
      double bestDeltaBleu = 0.0;
      unsigned int bestDirection = 0;
      for (unsigned int d = 0; d < directions.Size(); ++d) {
        if (!directions.ContainsAxis (d) ) {
          continue;
        }
        surface.Reset();
        ErrorSurface testSurface = surface;
        const std::vector<Sid> filtered = directions.FilteredLattices (d,
                                          this->lats.ids);
        this->LineOptimize (prev.first, directions.Get (d), testSurface, filtered);
        OptimizationResult current = this->MakeOptimizationResult (testSurface,
                                     directions.Get (d),
                                     prev);
        if (opts.pointTest) {
          Error debugError = ComputeError (this->refData, this->lats, current.first);
          if (debugError.GetError() != current.second.GetError() ) {
            tracer << "Direction: " << d
                   << " Incorrect Error Surface: "
                   << testSurface.GetOptimalGamma() << endl;
          }
        }
        if (!opts.writeSurface.empty() ) {
          testSurface.WriteErrorSurface (opts.writeSurface.data() );
        }
        this->LogLineOptimization (prev, current, directions.Get (d),
                                   testSurface.GetOptimalGamma() );
        double deltaBleu = current.second.GetError()
                           - prev.second.GetError();
        if (deltaBleu > bestDeltaBleu) {
          bestDeltaBleu = deltaBleu;
          bestDirection = d;
        }
        if (testSurface.GetUnbounded() ) {
          continue;
        }
        if (testSurface.boundaries.size() == 0) {
          if (opts.fullLog) {
            tracer <<
                   "    parameters not updated: no interval boundaries\n";
          }
          continue;
        }
        if (deltaBleu < opts.bleuThreshold) {
          if (opts.fullLog) {
            tracer << "    parameters not updated: delta bleu < "
                   << opts.bleuThreshold << '\n';
          }
          continue;
        }
        if (abs (testSurface.GetOptimalGamma() ) < opts.gammaThreshold) {
          if (opts.fullLog) {
            tracer <<
                   "    parameters not updated: change in gamma < "
                   << opts.gammaThreshold << '\n';
          }
          continue;
        }
        if (!mert.limits.check_in_range (d,
                                         testSurface.GetOptimalGamma() ) ) {
          continue;
        }
        surface = testSurface;
        prev = current;
      }
      if (prev.second.GetError() - start.second.GetError()
          < opts.bleuThreshold) {
        tracer <<
               "no direction gives sufficient improvement: delta bleu < "
               << opts.bleuThreshold << '\n';
        break;
      }
      UpdateDirection (start, prev, bestDeltaBleu, bestDirection,
                       directions);
      start = prev;
      if (!opts.fullLog) {
        tracer << "full powell iteration compleated with error: "
               << prev.second << '\n';
        std::cerr << std::fixed << std::setprecision (opts.printPrecision)
                  << VectorScale (prev.first) << endl;
      }
    }
    return make_pair (prev.first, prev.second.GetError() );
  }

  virtual ~PowellOptimizer() {
  }
};

#endif /* OPTIMIZE_H_ */
