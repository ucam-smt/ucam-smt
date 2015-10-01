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

#include <main.disambignffst.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <common-helpers.hpp>
#include <main.hpp>

// Simple wrapper around TopoFeaturesHelper class
// which handles lattice-specific options provided by the user
template<class ArcT>
struct DisambigFunctor {
  std::string const in_, out_;
  bool const detOut_;
  bool const minimize_;
  bool const exitOnFirstPassFailure_;
  bool const useOpenFst_;
  ucam::fsttools::SpeedStatsData ssd_;

  DisambigFunctor(std::string const &in, std::string const &out
                  , bool detOut, bool minimize, bool exitOnFirstPassFailure
		  , bool useOpenFst
                  )
      : in_(in)
      , out_(out)
      , detOut_(detOut)
      , minimize_(minimize)
      , exitOnFirstPassFailure_(exitOnFirstPassFailure)
      , useOpenFst_(useOpenFst)
  {}

  DisambigFunctor(DisambigFunctor const &df)
      : in_(df.in_)
      , out_(df.out_)
      , detOut_(df.detOut_)
      , minimize_(df.minimize_)
      , exitOnFirstPassFailure_(df.exitOnFirstPassFailure_)
      , ssd_(df.ssd_)
      , useOpenFst_(df.useOpenFst_)
  {}

  void operator()() {
    using namespace fst;
    using ucam::util::toString;
    using namespace ucam::fsttools;

    boost::scoped_ptr<fst::VectorFst<ArcT> >mfst
        (fst::VectorFstRead<ArcT> ( in_ ) );
    if (mfst->NumStates()) {
      if (detOut_)  // invert fst if selected by user.
        Invert(&*mfst);
      RmEpsilon(&*mfst);
      TopSort(&*mfst);

      std::string ns = toString(mfst->NumStates());
      ssd_.setTimeStart("disambig-" + out_ + ", NS=" + ns);
      if (!useOpenFst_) {
	if (!minimize_) {
	  LINFO("Only determinize");
	  TopoFeaturesHelper<ProjectDeterminizeAction> tfh(exitOnFirstPassFailure_);
	  tfh(&*mfst);
	} else { // Experimental option to play with.
	  LINFO("Determinize, Minimize and Push!");
	  TopoFeaturesHelper<ProjectDeterminizeMinimizePushAction> tfh(exitOnFirstPassFailure_);
	  tfh(&*mfst);
	}
	RmEpsilon(&*mfst); // take out epsilons created by reversals etc.
	ssd_.setTimeEnd("disambig-" + out_ + ", NS=" + ns);
	std::string nsd = toString(mfst->NumStates());
	FORCELINFO(out_ << ": NS=" << ns << ",NSD=" << nsd);
	LDBG_EXECUTE(mfst->Write("08-det-topo-final-rm.fst"));
      } else {
	// for quick comparisons. Note: not relabeling input epsilons.
	// affiliation lattices are epsilon-free FSTs.
#if OPENFSTVERSION >= 1004001
        LINFO("Openfst Determinize..."); 
	DeterminizeFstOptions<ArcT> dto;
#if OPENFSTVERSION >= 1005000
	dto.type = DETERMINIZE_DISAMBIGUATE;
#else
	dto.disambiguate_output = true;
#endif
	*mfst = DeterminizeFst<ArcT>(*mfst, dto);
#else
        LERROR("Openfst Determinize for non-functional FSTs is not supported (" 
         << OPENFSTVERSION << ")");
	exit(EXIT_FAILURE);
#endif
	ssd_.setTimeEnd("disambig-" + out_ + ", NS=" + ns);
      }
    } else {
      ssd_.setTimeStart("disambig-" + out_ + ", NS=0");
      ssd_.setTimeEnd("disambig-" + out_ + ", NS=0");
    }
    if (detOut_) // invert back
      Invert(&*mfst);

    FstWrite<StdArc> (*mfst, out_ );
    ssd_.write(std::cerr);
  }
};


// Templated method that runs over a list of FST files
// loads, runs disambiguation and stores.
template<class ArcT, class ThreadPoolT >
inline void run(ucam::util::RegistryPO const &rg
                , ThreadPoolT &tp) {
  using namespace HifstConstants;
  using namespace ucam::util;

  PatternAddress<unsigned> pi (rg.get<std::string>(kInput) );
  PatternAddress<unsigned> po (rg.get<std::string>(kOutput) );
  bool detOut = rg.getBool(kDeterminizeOutput);
  bool minimize = rg.getBool(kMinimize);
  bool exitOnFirstPassFailure = rg.getBool(kExitOnFirstPassFailure);
  bool useOpenFst = rg.getBool(kUseOpenFst);
  using namespace fst;
  for ( IntRangePtr ir (IntRangeFactory ( rg, kRangeOne ) );
        !ir->done();
        ir->next() ) {
    DisambigFunctor<ArcT> df(pi(ir->get()), po(ir->get())
                             , detOut, minimize, exitOnFirstPassFailure
			     , useOpenFst);
    tp(df);
  }
};

// Main function call this overloaded method (see main.hpp for details)
// Determine semiring, multithreading and kick off disambiguation of
// (potentially non-funcitonal) FSTs using topological features
void ucam::util::MainClass::run() {
  using namespace HifstConstants;
  using namespace ucam::util;
  std::string const arctype =rg_->get<std::string>(kHifstSemiring);
  unsigned nthreads = (rg_->exists( kNThreads) )
    ? rg_->get<unsigned>(kNThreads) : 0;

  if (arctype == kHifstSemiringStdArc) {
    if (nthreads)  { // even the single thread is in the trivial pool
      FORCELINFO("Multithreading with " << nthreads << " threads");
      TrivialThreadPool tp(nthreads);
      ::run<fst::StdArc, TrivialThreadPool>(*rg_, tp);
    } else { // not defined, will run single thread without the trivial pool
      NoThreadPool ntp;
      ::run<fst::StdArc, NoThreadPool>(*rg_, ntp);
    }
  } else {
    LERROR("Unknown semiring!");
    exit(EXIT_FAILURE);
  }
}
