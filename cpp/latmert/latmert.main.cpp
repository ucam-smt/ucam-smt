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

#include "LatMertMain.h"
#include "DebugMert.h"
#include "TGMert.h"
#include "ParamsConfig.h"
#include "ErrorSurface.h"
#include "RefsData.h"
#include "CommonFlags.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <fst/fstlib.h>
#include <boost/tuple/tuple_comparison.hpp>

////////////////////////////////
// Parameter Flag Definitions //
////////////////////////////////

DEFINE_string (params_min, "",
               "vector of minimum changes accepted during line optimization");
DEFINE_string (params_max, "",
               "vector of maximum changes accepted during line optimization");
DEFINE_string (direction, "", "direction(s)");
DEFINE_string (write_surface, "", "path to write error surface");
DEFINE_string (write_parameters, "", "path to write tuned parameters");
DEFINE_string (algorithm, "", "Line search algorithm to use");
DEFINE_string (search, "powell", "Algorithm to pick search direction");

DEFINE_double (gamma_threshold, 0.000005, "gamma parameter update threshold");
DEFINE_double (bleu_threshold, 0.000001, "BLEU parameter update threshold");

DEFINE_int32 (print_precision, 6,
              "print precision for vector weight components");
DEFINE_int32 (lattice_cutoff, 1,
              "The number of lattices necessary for tuning a feature parameter");
DEFINE_int32 (random_directions, 10, "The number of random directions");

DEFINE_bool (verbose, false, "enable verbose output");
DEFINE_bool (cache_lattices, false,
             "load all lattices into memory prior to MERT");
DEFINE_bool (normalize_parameters, true,
             "normalize final tuned parameters with respect to the first column");
DEFINE_bool (prune_stats, false, "print number of arcs pruned");
DEFINE_bool (ignore_gsf, false, "ignore grammar scale factor");
DEFINE_bool (no_skip, false,
             "do not skip lattices if they do not contain features");
DEFINE_bool (full_log, false,
             "print an old style log with feature vectors displayed");
DEFINE_bool (point_test, false,
             "check that each optimised parameter gives the same error as hypotheses found by shortest path");
DEFINE_bool (random_axes, false, "use axes along with random directions");

static const char* CMD_LINE_TOKEN_AXES = "axes";

template<class Algo, class ErrorSurface>
void PowellOptimizer<Algo, ErrorSurface>::Init (int dim) {
  std::string pattern = FLAGS_direction.data();
  if (pattern.find (CMD_LINE_TOKEN_AXES) == 0) {
    directions.Resize (dim, this->lats);
  } else {
    directions.Set (InitializeVectors (pattern), this->lats);
  }
  tracer << directions.Size() << " optimization directions(s)\n";
}

template<class Algo, class ErrorSurface>
void RandomOptimizer<Algo, ErrorSurface>::Init (int dim) {
  dirGen = RandDirGenerator (dim);
  this->dim = dim;
  directions = FLAGS_random_directions;
  useAxes = FLAGS_random_axes;
}

std::map<boost::tuple<std::string, std::string, std::string> , Optimizer *>
CreateOptimizers() {
  std::map<boost::tuple<std::string, std::string, std::string> , Optimizer *>
  optimizers;
  optimizers[boost::make_tuple ("bleu", "lmert",
                                "powell")] = new PowellOptimizer <
  LMertAlgorithm, ErrorSurface<IntegerEncRefs> > ();
  optimizers[boost::make_tuple ("bleu", "tgmert",
                                "powell")] = new PowellOptimizer <
  TGMertAlgorithm<TupleArc>, ErrorSurface<IntegerEncRefs> > ();
  optimizers[boost::make_tuple ("bleu", "debug",
                                "powell")] = new PowellOptimizer <
  DebugMertAlgorithm, ErrorSurface<IntegerEncRefs> > ();
  optimizers[boost::make_tuple ("bleu", "lmert",
                                "random")] = new RandomOptimizer <
  LMertAlgorithm, ErrorSurface<IntegerEncRefs> > ();
  optimizers[boost::make_tuple ("bleu", "tgmert",
                                "random")] = new RandomOptimizer <
  TGMertAlgorithm<TupleArc>, ErrorSurface<IntegerEncRefs> > ();
  optimizers[boost::make_tuple ("bleu", "debug",
                                "random")] = new RandomOptimizer <
  DebugMertAlgorithm, ErrorSurface<IntegerEncRefs> > ();
  return optimizers;
}

std::map<boost::tuple<std::string, std::string, std::string> , Optimizer *>
optimizers =
  CreateOptimizers();

void WriteTunedParameters (const PARAMS& best,
                           const std::string& filename, const bool normalize) {
  std::ofstream ofs (filename.c_str() );
  if (!ofs.good() ) {
    cerr << "ERROR: unable to write file " << filename << '\n';
    exit (1);
  }
  PARAMS final = best;
  if (opts.scaleParams) {
    final = VectorScale (final);
  }
  tracer << "writing final parameters to file: " << filename << '\n';
  ofs << std::fixed << std::setprecision (opts.printPrecision) << final << '\n';
  ofs.close();
  tracer << "  " << std::fixed << std::setprecision (opts.printPrecision) << final
         << '\n';
}

int main (int argc, char** argv) {
  tracer << argv[0] << " starting\n";
  std::string usage = "latmert\n\n  Usage:";
  usage += argv[0];
  InitFst (usage.c_str(), &argc, &argv, true);
  if (FLAGS_lambda.empty() ) {
    cerr << "ERROR: mandatory parameter not specified: 'lambda'\n";
    return 1;
  }
  if (FLAGS_direction.empty() ) {
    cerr << "ERROR: mandatory parameter not specified: 'direction'\n";
    return 1;
  }
  if (FLAGS_algorithm.empty() ) {
    cerr << "ERROR: mandatory parameter not specified: 'algorithm'\n";
    return 1;
  }
  //tracer << "M=" << PARAMS::VECTOR_DIMENSIONS << " dimensions" << '\n';
  // Set options
  opts.bleuThreshold = FLAGS_bleu_threshold;
  opts.gammaThreshold = FLAGS_gamma_threshold;
  opts.verbose = FLAGS_verbose;
  opts.printPrecision = FLAGS_print_precision;
  opts.latticeCutoff = FLAGS_lattice_cutoff;
  opts.writeSurface = FLAGS_write_surface;
  opts.scaleParams = FLAGS_normalize_parameters;
  opts.ignoreGsf = FLAGS_ignore_gsf;
  opts.useCache = FLAGS_cache_lattices;
  opts.noSkip = FLAGS_no_skip;
  opts.fullLog = FLAGS_full_log;
  opts.pointTest = FLAGS_point_test;
  PARAMS lambda = InitializeVectors (FLAGS_lambda.data() ) [0];
  unsigned int dim = lambda.size();
  PARAMS params_min;
  PARAMS params_max;
  if (!FLAGS_params_min.empty() ) {
    params_min = fst::ParseParamString<double, std::vector<double> > (
                   ReadWeight (FLAGS_params_min.data() ) );
    if (params_min.size() != dim) {
      cerr << "Params min dimensionality does not match param vector "
           << endl;
      exit (1);
    }
  }
  if (!FLAGS_params_max.empty() ) {
    params_max = fst::ParseParamString<double, std::vector<double> > (
                   ReadWeight (FLAGS_params_max.data() ) );
    if (params_max.size() != dim) {
      cerr << "Params max dimensionality does not match param vector "
           << endl;
      exit (1);
    }
  }
  tracer << "dimensions = " << dim << '\n';
  mert.limits.Initialize (params_min, params_max);
  tracer << "params min = " << params_min << '\n';
  tracer << "params max = " << params_max << '\n';
  tracer << "gamma threshold = " << FLAGS_gamma_threshold << '\n';
  tracer << "bleu threshold = " << FLAGS_bleu_threshold << '\n';
  tracer << "weight type = " << TupleW::Type() << '\n';
  boost::tuple<std::string, std::string, std::string> key = boost::make_tuple (
        FLAGS_error_function,
        FLAGS_algorithm, FLAGS_search);
  Optimizer* optimizer = optimizers[key];
  if (optimizer == 0) {
    tracer << "Unrecognized Optimizer\t" << FLAGS_error_function << "\t" <<
           FLAGS_algorithm << "\t" << FLAGS_search << endl;
    exit (1);
  }
  optimizer->InitTuneSet (FLAGS_cache_lattices);
  std::vector<std::string> refFilenames = InitRefDataFilenames (argc, argv);
  optimizer->LoadRefData (refFilenames);
  optimizer->Init (dim);
  std::string startScore = optimizer->ComputeError (lambda);
  tracer << "starting point bleu=" << startScore << endl;
  pair<PARAMS, double> tuned = (*optimizer) (lambda);
  tracer << "optimization result:\n";
  if (opts.fullLog) {
    tracer << "  start: " << lambda << " " << startScore << endl;
    tracer << "  final: " << tuned.first << " " << tuned.second << endl;
  } else {
    tracer << "  start: " << startScore << endl;
    tracer << "  final: " << tuned.second << endl;
  }
  if (!FLAGS_write_parameters.empty() ) {
    WriteTunedParameters (tuned.first, FLAGS_write_parameters,
                          FLAGS_normalize_parameters);
  }
  return 0;
}
