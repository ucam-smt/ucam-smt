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
 * \brief Hifst main entry file.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

///Include all necessary headers here.
#include <main.hifst.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.hifst.hpp>
#include <kenlmdetect.hpp>

/**
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks     Main function. Runs hifst tool.
 * First parses program options with boost, then loads and chains several task classes.
 * Finally, kick off translation for a range of sentences.
 */

int
main ( int argc, const char *argv[] ) {
  using ucam::util::Runner3;
  using ucam::hifst::HifstTaskData;
  using ucam::hifst::SingleThreadedHifstTask;
  using ucam::hifst::MultiThreadedHifstTask;
  using ucam::hifst::HifstServerTask;
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
    ( Runner3<SingleThreadedHifstTask<>
      , MultiThreadedHifstTask<>
      , HifstServerTask<> > ( rg ) ) ();
    break;
  case lm::ngram::REST_PROBING:
    ( Runner3<SingleThreadedHifstTask<HifstTaskData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel>
      , MultiThreadedHifstTask<HifstTaskData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel>
      , HifstServerTask<HifstTaskData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel> >
      ( rg ) ) ();
    break;
  case lm::ngram::TRIE:
    ( Runner3<SingleThreadedHifstTask<HifstTaskData<lm::ngram::TrieModel>, lm::ngram::TrieModel>
      , MultiThreadedHifstTask<HifstTaskData<lm::ngram::TrieModel>, lm::ngram::TrieModel>
      , HifstServerTask<HifstTaskData<lm::ngram::TrieModel>, lm::ngram::TrieModel> >
      ( rg ) ) ();
    break;
  case lm::ngram::QUANT_TRIE:
    ( Runner3<SingleThreadedHifstTask<HifstTaskData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel>
      , MultiThreadedHifstTask<HifstTaskData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel>
      , HifstServerTask<HifstTaskData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel> >
      ( rg ) ) ();
    break;
  case lm::ngram::ARRAY_TRIE:
    ( Runner3<SingleThreadedHifstTask<HifstTaskData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel>
      , MultiThreadedHifstTask<HifstTaskData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel>
      , HifstServerTask<HifstTaskData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel> >
      ( rg ) ) ();
    break;
  case lm::ngram::QUANT_ARRAY_TRIE:
    ( Runner3<SingleThreadedHifstTask<HifstTaskData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel>
      , MultiThreadedHifstTask<HifstTaskData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel>
      , HifstServerTask<HifstTaskData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel> >
      ( rg ) ) ();
    break;
  }
  FORCELINFO ( argv[0] << " ends!" );
  return 0;
}
