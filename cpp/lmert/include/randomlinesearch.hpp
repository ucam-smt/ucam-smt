#ifndef RANDOMLINESEARCH_HPP
#define RANDOMLINESEARCH_HPP

#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_on_sphere.hpp>
#include <boost/random/variate_generator.hpp>
#include <ctime>

template <class Arc>
class RandomLineSearch {
public:
  RandomLineSearch(ucam::util::RegistryPO const& rg, TuneSet<Arc>& ts, BleuScorer& bs, PARAMS32 lambda) : 
    lambda_(lambda), fdim_(lambda.size()), gammaMin_(rg.get<float>("min_gamma")), minBleuGain_(rg.get<float>("min_bleu_gain")), seed_(rg.get<int>("random_seed")), numRandDirs_(rg.get<int>("num_random_directions")) {
    rand_gen.seed(seed_);
    if (numRandDirs_ <= 0)
      numRandDirs_ = 2 * fdim_;
    iBleu_ = ts.ComputeBleu(bs, lambda);
    std::cerr << "Lambda: " << lambda_ << "(dim " << fdim_ << ")" << std::endl << "Initial Bleu: " << iBleu_ << std::endl;    
    optBleu_ = iBleu_;
    while (true) {
      GenerateDirections();
      double optGamma = 0.0;
      int optDirection;
      Bleu pBleu(optBleu_);
      for (int i=0; i<directions_.size(); i++) {
	LineOptimize< Arc > lopt(rg, ts, bs, lambda_, directions_[i]);
	bool smallGamma = (fabs(lopt.OptimalGamma()) < gammaMin_);
	bool smallBleuGain = (lopt.OptimalBleu().m_bleu < optBleu_.m_bleu + minBleuGain_);
	if (!smallBleuGain && !smallGamma) { // TODO: add check for unbounded
	  optBleu_ = lopt.OptimalBleu();
	  optGamma = lopt.OptimalGamma();
	  optDirection = i;
	} 
	std::cerr << "Direction["<< i << "]: Opt Bleu: " << lopt.OptimalBleu() << (smallBleuGain ? " - too small; " : "; ") 
		  << "Gamma: " << lopt.OptimalGamma() << (smallGamma ? " - too small" : "") << std::endl;
      }
      std::cerr << "Full random iteration completed. Opt Bleu: " << optBleu_ << "; Opt Gamma: " << optGamma << (optGamma < gammaMin_ ? " - too small" : "") << std::endl;
      if (optBleu_.m_bleu < pBleu.m_bleu + minBleuGain_) {
	std::cerr << "Bleu gain less than threshold. Exiting." << std::endl;
	optBleu_ = pBleu;
	break;
      }
      UpdateLambda(directions_[optDirection], optGamma);
      std::cerr << "Lambda: " << lambda_ << std::endl;
    }
    std::cerr << "Initial Bleu: " << iBleu_ << std::endl << "Final Bleu:   " << optBleu_ << std::endl << "Final Lambda: " << lambda_ << std::endl;
    if (rg.get<string>("write_params").length()) {
      std::ofstream ofs(rg.get<string>("write_params").c_str());
      if (!ofs.good()) {
	std::cerr << "Can't write to " << rg.get<string>("write_params") << std::endl;
	exit(1);
      }
      std::cerr << "Writing final Lambda to " << rg.get<string>("write_params") << std::endl;
      ofs << lambda_ << std::endl;
      ofs.close();
    }
  }

private:

  void GenerateDirections() {
    directions_.clear();
    for (int i=0; i<fdim_; i++) {
      PARAMS32 dir (fdim_, 0.0);
      dir[i] = 1.0;
      directions_.push_back( dir );
    }
    boost::uniform_on_sphere<float> unif_sphere (fdim_);
    std::cerr << "Generating " << numRandDirs_ << " random directions" << std::endl;
    for (int i=0; i<numRandDirs_; i++) {
      boost::random::variate_generator<boost::random::mt19937&, boost::random::uniform_on_sphere<float> > sg(rand_gen, unif_sphere);
      directions_.push_back(sg());
      //      std::cerr << directions_.back() << std::endl;
    }
  }

  void UpdateLambda(PARAMS32 direction, double gamma) {
    if (gamma == 0.0)
      return;
    for (int i=0; i<fdim_; i++) {
      lambda_[i] = lambda_[i] + gamma*direction[i];
    }
  }

  boost::random::mt19937 rand_gen;
  vector< PARAMS32 > directions_;
  PARAMS32 lambda_;
  Bleu optBleu_;
  Bleu iBleu_;
  float minBleuGain_;
  float gammaMin_;
  int fdim_;
  int seed_;
  int numRandDirs_;
};

#endif
