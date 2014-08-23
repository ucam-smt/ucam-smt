#ifndef MAIN_LMERT_HPP
#define MAIN_LMERT_HPP

#include <global_incls.hpp>
#include <custom_assert.hpp>
#include <global_decls.hpp>
#include <global_funcs.hpp>
#include <fst/fstlib.h>
#include <fst/script/print.h>
#include <logger.hpp>
#include <params.hpp>
#include <tropical-sparse-tuple-weight-incls.h>
#include <tropical-sparse-tuple-weight.h>
#include <tropical-sparse-tuple-weight-decls.h>
#include <szfstream.hpp>
#include <registrypo.hpp>
#include <taskinterface.hpp>
#include <range.hpp>
#include <addresshandler.hpp>
#include <fstio.hpp>
#include <bleu.hpp>
#include <tuneset.hpp>
#include <constants-fsttools.hpp>

#include <lmert.hpp>
#include <lineoptimize.hpp>
#include <randomlinesearch.hpp>

#include <main.lmert.init_param_options.hpp>

namespace ucam {
namespace lmert {
  
ucam::fsttools::PARAMS32 GetLambda ( ucam::util::RegistryPO const& rg ) {
  std::string tuplearcWeights = rg.get<std::string> ( HifstConstants::kLmertInitialParams );

  if ( tuplearcWeights.empty() )
    LERROR ( "weights not set" );

  std::string ftok ( "file:" );
  std::size_t found = tuplearcWeights.find ( ftok );

  if ( found == std::string::npos )
    return ucam::util::ParseParamString<float> ( tuplearcWeights );

  tuplearcWeights.erase ( 0, ftok.length() );
  std::ifstream ifs ( tuplearcWeights.c_str() );

  if ( !ifs.good() ) {
    LERROR ( "Unable to open " << tuplearcWeights );
    exit ( EXIT_FAILURE );
  }

	std::string p;
  getline ( ifs, p );
  ifs.close();
  return ucam::util::ParseParamString<float> ( p );
};

}}  // endif
#endif
