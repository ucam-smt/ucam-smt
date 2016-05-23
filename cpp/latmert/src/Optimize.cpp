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

#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include "Optimize.h"
#include <ctime>
#include <boost/random/uniform_on_sphere.hpp>
#include <boost/random/variate_generator.hpp>

typedef unsigned int Sid; // Sentence ID

typedef std::vector<Sid> SentenceList;

MERT mert;

DEFINE_int32 (threads, 1, "Number of threads used for line search");

DEFINE_int64 (seed, 0, "Random direction seed");

unsigned int GetSeed() {
  static unsigned int seed = 0;
  if (seed != 0) {
    return seed;
  }
  if (FLAGS_seed == 0) {
    seed = static_cast<unsigned int> (std::time (0) );
  } else {
    seed = (unsigned int) FLAGS_seed;
  }
  tracer << "Seed used for random directions: " << seed << std::endl;
  return seed;
}

RandDirGenerator::RandDirGenerator (unsigned int dim) : dim (dim) {
  rand_gen.seed (GetSeed() );
}

PARAMS RandDirGenerator::GenerateDirection() {
  boost::uniform_on_sphere<double> unif_sphere (dim);
  boost::random::variate_generator<boost::random::mt19937&, boost::random::uniform_on_sphere<double> >
  sphereGen (rand_gen, unif_sphere);
  return sphereGen();
}

PARAMS ComputeFinalPoint (const PARAMS& lambda, const PARAMS& direction,
                          const double gamma) {
  PARAMS vv (lambda.size() );
  for (unsigned int k = 0; k < lambda.size(); ++k) {
    vv[k] = lambda[k] + gamma * direction[k];
  }
  return vv;
}
/*
 * Scale a PARAMS by the a single parameter value. Designed to combat numerical instability.
 */

PARAMS VectorScale (const PARAMS& vw, const unsigned int k) {
  const double normalization = fabs (vw[k]);
  PARAMS vv = vw;
  if (abs (normalization) > kDoubleDelta) {
    for (unsigned int i = 0; i < vw.size(); ++i) {
      vv[i] = vv[i] / normalization;
    }
  }
  return vv;
}
/*
 template<class T>
 OptimizationResult OptimizeRandom(const PARAMS& startPoint, int noOfDirctions, const Algorithm<T>& algo) {

 }
 */

Directions::Directions() :
  hasDirection (0) {
}

void Directions::Resize (unsigned int dim, TuneSet& lats) {
  this->dim = dim;
  current.resize (dim);
  if (hasDirection != 0) {
    delete hasDirection;
  }
  hasDirection = new bool[dim];
  for (unsigned int i = 0; i < dim; ++i) {
    current[i] = 0.0;
    hasDirection[i] = false;
  }
  feature_filter.clear();
  std::unordered_set<unsigned int> total_features;
  unordered_map<unsigned int, std::vector<Sid> > latticesByFeature;
  for (std::vector<Sid>::const_iterator sit = lats.ids.begin();
       sit != lats.ids.end(); ++sit) {
    //sparse_hash_set<unsigned int> features;
    TupleArcFst* fst = lats.GetVectorLattice (*sit, opts.useCache);
    for (fst::StateIterator<TupleArcFst> si (*fst); !si.Done(); si.Next() ) {
      TupleArcFst::StateId state_id = si.Value();
      for (fst::MutableArcIterator<TupleArcFst> ai (fst, si.Value() );
           !ai.Done(); ai.Next() ) {
        const TupleW w = ai.Value().weight;
        for (fst::SparseTupleWeightIterator<FeatureWeight, int> it (w);
             !it.Done(); it.Next() ) {
          latticesByFeature[it.Value().first - 1].push_back (*sit);
        }
      }
    }
    if (!opts.useCache) {
      delete fst;
    }
  }
  for (unordered_map<unsigned int, std::vector<Sid> >::const_iterator it =
         latticesByFeature.begin(); it != latticesByFeature.end(); ++it) {
    if (it->second.size() >= opts.latticeCutoff) {
      hasDirection[it->first] = true;
      total_features.insert (it->first);
      for (std::vector<Sid>::const_iterator sit = it->second.begin();
           sit != it->second.end(); ++sit) {
        feature_filter[*sit].insert (it->first);
      }
    }
  }
  tracer << "lattices contain " << total_features.size() << " features"
         << std::endl;
}

const PARAMS& Directions::Get (unsigned int axis) {
  current[prev] = 0;
  prev = axis;
  if (directions.count (axis) > 0) {
    return directions[axis];
  }
  current[axis] = 1;
  return current;
}

void Directions::Set (unsigned int axis, PARAMS direction) {
  directions[axis] = direction;
}

void Directions::Set (std::vector<PARAMS> batch, TuneSet& lats) {
  directions.clear();
  for (unsigned int i; i < batch.size(); ++i) {
    directions[i] = batch[i];
  }
  dim = batch.size();
  Resize (dim, lats);
}

unsigned int Directions::Size() {
  return dim;
}

bool Directions::ContainsAxis (Sid sid, unsigned int axis) {
  if (directions.count (axis) > 0) {
    return true;
  }
  const std::unordered_set<unsigned int>& features = feature_filter[sid];
  std::unordered_set<unsigned int>::const_iterator it = features.find (axis);
  bool containsAxis = it != features.end();
  return containsAxis;
}

const std::vector<Sid> Directions::FilteredLattices (unsigned int axis,
    const std::vector<Sid>& lats) {
  std::vector<Sid> filtered;
  for (std::vector<Sid>::const_iterator sit = lats.begin();
       sit != lats.end(); ++sit) {
    if (ContainsAxis (*sit, axis) && !opts.noSkip) {
      if (opts.verbose) {
        tracer << "Skipping axis: " << axis << " for sentence "
               << *sit << '\n';
      }
      filtered.push_back (*sit);
    }
  }
  return filtered;
}

bool Directions::ContainsAxis (unsigned int axis) {
  if (directions.count (axis) > 0) {
    return true;
  }
  return hasDirection[axis];
}

int GetNoOfThreads() {
  return FLAGS_threads;
}
