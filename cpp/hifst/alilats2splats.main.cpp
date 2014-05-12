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

#define HIFST

/**
 * \file
 * \brief Alignment lattices to sparse vector weight lattices
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

///Include all necessary headers here.
#include <main.alilats2splats.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.alilats2splats.hpp>
#include <kenlmdetect.hpp>

/**
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks     Main function. Runs alilats2splats tool. See main-run.alilats2splats.hpp
 */

int
main ( int argc, const char *argv[] ) {
  using ucam::util::Runner2;
  using ucam::hifst::SingleThreadedAliLatsToSparseVecLatsTask;
  using ucam::hifst::MultiThreadedAliLatsToSparseVecLatsTask;
  using ucam::hifst::AlilatsToSparseWeightLatsData;
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n====================="
                         , "=====================" ) );
  // Detect here kenlm binary type
  // it's a bit ugly this way of initializing the correct kenlm handler
  lm::ngram::ModelType kenmt = ucam::util::detectkenlm (rg.getVectorString (
                                 HifstConstants::kLmLoad, 0) );
  switch (kenmt) {
  case lm::ngram::PROBING:
    ( Runner2<SingleThreadedAliLatsToSparseVecLatsTask<>
      , MultiThreadedAliLatsToSparseVecLatsTask<>  > ( rg ) ) ();
    break;
  case lm::ngram::REST_PROBING:
    ( Runner2<SingleThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel>
      , MultiThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel>  >
      ( rg ) ) ();
  case lm::ngram::TRIE:
    ( Runner2<SingleThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::TrieModel>, lm::ngram::TrieModel>
      , MultiThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::TrieModel>, lm::ngram::TrieModel>  >
      ( rg ) ) ();
  case lm::ngram::QUANT_TRIE:
    ( Runner2<SingleThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel>
      , MultiThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel>  >
      ( rg ) ) ();
  case lm::ngram::ARRAY_TRIE:
    ( Runner2<SingleThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel  >
      , MultiThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel>  >
      ( rg ) ) ();
  case lm::ngram::QUANT_ARRAY_TRIE:
    ( Runner2<SingleThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel>
      , MultiThreadedAliLatsToSparseVecLatsTask<AlilatsToSparseWeightLatsData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel> >
      ( rg ) ) ();
    break;
  }
  FORCELINFO ( argv[0] << " ends!" );
  return 0;
}
