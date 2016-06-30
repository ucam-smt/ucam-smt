#ifndef LMERT_SINGLELINESEARCH_HPP
#define LMERT_SINGLELINESEARCH_HPP

#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_on_sphere.hpp>
#include <boost/random/variate_generator.hpp>
#include <ctime>

#include <constants-lmert.hpp>

namespace ucam {
namespace lmert {

template <class Arc>
class SingleLineSearch {
public:
  SingleLineSearch ( ucam::util::RegistryPO const& rg,
		     ucam::fsttools::TuneSet<Arc>& ts,
                     ucam::fsttools::BleuScorer& bs,
		     ucam::fsttools::PARAMS32 const &lambda,
		     ucam::fsttools::PARAMS32 const &direction
		     ) {
    ucam::fsttools::Bleu iBleu = ts.ComputeBleu ( bs, lambda );
    FORCELINFO("Lambda: " << ucam::util::printout(lambda));
    LineOptimize< Arc > lopt ( rg, ts, bs, lambda, direction );
    FORCELINFO("Initial Bleu: " << iBleu);
    FORCELINFO("Final Gamma: " << lopt.OptimalGamma());
    FORCELINFO("Final Bleu:   " << lopt.OptimalBleu());
    FORCELINFO("Final Lambda: " << ucam::util::printout(lambda) );
    ucam::util::WriteParamFile(rg.get<string>(HifstConstants::kLmertWriteParams), lambda);
  }

};


}}  // end namespaces

#endif
