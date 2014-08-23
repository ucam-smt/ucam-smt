#ifndef LMERT_RANDOMLINESEARCH_HPP
#define LMERT_RANDOMLINESEARCH_HPP

#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_on_sphere.hpp>
#include <boost/random/variate_generator.hpp>
#include <ctime>

#include <constants-lmert.hpp>

namespace ucam {
namespace lmert {

template <class Arc>
class RandomLineSearch {
 public:
  RandomLineSearch ( ucam::util::RegistryPO const& rg,
										 ucam::fsttools::TuneSet<Arc>& ts,
                     ucam::fsttools::BleuScorer& bs,
										 ucam::fsttools::PARAMS32 const &lambda ) :
    lambda_ ( lambda ), fdim_ ( lambda.size() ),
    gammaMin_ ( rg.get<float> ( HifstConstants::kLmertMinGamma ) ),
    minBleuGain_ ( rg.get<float> ( HifstConstants::kLmertMinBleuGain ) ),
    seed_ ( rg.get<int> ( HifstConstants::kLmertRandomSeed ) ),
    numRandDirs_ ( rg.get<int> ( HifstConstants::kLmertNumRandomDirections ) ) {

    rand_gen.seed ( seed_ );

    if ( numRandDirs_ <= 0 )
      numRandDirs_ = 2 * fdim_;

    iBleu_ = ts.ComputeBleu ( bs, lambda );
		LINFO("Lambda: " << ucam::util::printout(lambda_) 
					<< "(dim " << fdim_ << ") Initial Bleu: " << iBleu_);
    optBleu_ = iBleu_;

    while ( true ) {
      GenerateDirections();
      double optGamma = 0.0;
      int optDirection;
      Bleu pBleu ( optBleu_ );

      for ( int i = 0; i < directions_.size(); i++ ) {
        LineOptimize< Arc > lopt ( rg, ts, bs, lambda_, directions_[i] );
        bool smallGamma = ( fabs ( lopt.OptimalGamma() ) < gammaMin_ );
        bool smallBleuGain = ( lopt.OptimalBleu().m_bleu < optBleu_.m_bleu +
                               minBleuGain_ );

        if ( !smallBleuGain && !smallGamma ) { // TODO: add check for unbounded
          optBleu_ = lopt.OptimalBleu();
          optGamma = lopt.OptimalGamma();
          optDirection = i;
        }

        LINFO("Direction[" << i << "]: Opt Bleu: " << lopt.OptimalBleu() <<
							( smallBleuGain ? " - too small; " : "; " )
							<< "Gamma: " << lopt.OptimalGamma() << ( smallGamma ? " - too small" : "" ));

      }

      LINFO("Full random iteration completed. Opt Bleu: " << optBleu_ <<
						"; Opt Gamma: " << optGamma << ( optGamma < gammaMin_ ? " - too small" : "" ) );

      if ( optBleu_.m_bleu < pBleu.m_bleu + minBleuGain_ ) {
        FORCELINFO("Bleu gain less than threshold. Exiting.");
        optBleu_ = pBleu;
        break;
      }

      UpdateLambda ( directions_[optDirection], optGamma );
      LINFO("Lambda: " << ucam::util::printout(lambda_) );
    }

    LINFO("Initial Bleu: " << iBleu_ << std::endl << "Final Bleu:   " <<
					optBleu_ << std::endl << "Final Lambda: " << ucam::util::printout(lambda_) );

		std::string writeParamsFileName = rg.get<string> (HifstConstants::kLmertWriteParams );
    if ( writeParamsFileName.length() ) {
      std::ofstream ofs ( writeParamsFileName.c_str() );

      if ( !ofs.good() ) {
        LERROR("Can't write to " << writeParamsFileName);
        exit ( EXIT_FAILURE );
      }

      LINFO("Writing final Lambda to " << writeParamsFileName );
      ofs << ucam::util::printout(lambda_) << std::endl;
      ofs.close();
    }
  }

 private:

  void GenerateDirections() {
    directions_.clear();

    for ( int i = 0; i < fdim_; i++ ) {
      PARAMS32 dir ( fdim_, 0.0 );
      dir[i] = 1.0;
      directions_.push_back ( dir );
    }

    boost::uniform_on_sphere<float> unif_sphere ( fdim_ );
    FORCELINFO("Generating " << numRandDirs_ << " random directions");

    for ( int i = 0; i < numRandDirs_; i++ ) {
      boost::random::variate_generator<boost::random::mt19937&,
																			 boost::random::uniform_on_sphere<float> >
      sg ( rand_gen, unif_sphere );
      directions_.push_back ( sg() );
      //      std::cerr << directions_.back() << std::endl;
    }
  }

  void UpdateLambda ( PARAMS32 direction, double gamma ) {
    if ( gamma == 0.0 )
      return;

    for ( int i = 0; i < fdim_; i++ ) {
      lambda_[i] = lambda_[i] + gamma * direction[i];
    }
  }

	typedef ucam::fsttools::PARAMS32 PARAMS32;
	typedef ucam::fsttools::Bleu Bleu;
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


}}  // end namespaces

#endif
