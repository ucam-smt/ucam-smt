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

#ifndef APPLYLMTASK_KENLMTYPE_HPP
#define APPLYLMTASK_KENLMTYPE_HPP

/**
 * \file
 * \brief Wrapper to ApplyLanguageModelOnTheFly to apply different kenlm models
 * \date 16-2-2015
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

template<class Arc, template<class> class MakeWeightT>
inline fst::ApplyLanguageModelOnTheFlyInterface<Arc> *
assignKenLmHandler(util::RegistryPO const &rg
		   , std::string const &lmkey
		   , std::tr1::unordered_set<typename Arc::Label> &epsilons
		   , KenLMData const &klm
		   , MakeWeightT<Arc> &mw
		   , bool useNaturalLog
		   , unsigned offset = 0) {
  using namespace lm::ngram;
  typedef lm::np::Model NplmModel;
  // Detect here kenlm binary type
  std::string file = rg.getVectorString (lmkey, offset) ;
  int  kenmt = ucam::util::detectkenlm(file);

  switch (kenmt) {
  case PROBING:
    return new  fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, ProbingModel>
      (dynamic_cast<ProbingModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
  case REST_PROBING:
    return new  fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, RestProbingModel >
      (dynamic_cast<RestProbingModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
  case TRIE:
    return new  fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, TrieModel >
      (dynamic_cast<TrieModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
  case QUANT_TRIE:
    return new  fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, QuantTrieModel >
      (dynamic_cast<QuantTrieModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
  case ARRAY_TRIE:
    return new  fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, ArrayTrieModel >
      (dynamic_cast<ArrayTrieModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
  case QUANT_ARRAY_TRIE:
    return new  fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, QuantArrayTrieModel >
      (dynamic_cast<QuantArrayTrieModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
  case util::KENLM_NPLM:
 #ifdef WITH_NPLM
    return new fst::ApplyLanguageModelOnTheFly<Arc, MakeWeightT<Arc>, NplmModel > 
      (dynamic_cast<NplmModel &>(*klm.model), epsilons,useNaturalLog, klm.lmscale, klm.lmwp, klm.idb, mw);
#endif
    LERROR("Unsuported format: KENLM_NPLM. Did you compile NPLM library?");
     exit(EXIT_FAILURE);
  default:
    // bad news if it reaches this point, as format should default to probing
    LERROR("Programmer mistake -- (task.applylm.kenlmtype.hpp)");
    exit(EXIT_FAILURE);
  }
  return NULL;
};


}} // end namespaces

#endif
