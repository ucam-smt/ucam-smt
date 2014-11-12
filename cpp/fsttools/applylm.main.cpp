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

/**
 * \file
 * \brief Main file for applylm tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <main.applylm.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.applylm.hpp>
#include <kenlmdetect.hpp>

/*
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks     Main function. Runs applylm tool -- apply language models using kenlm
 * Multithreaded version included, triggered by nthread option
 */

int main ( int argc, const char* argv[] ) {
  using ucam::util::Runner2;
  using ucam::fsttools::SingleThreadedApplyLanguageModelTask;
  using ucam::fsttools::MultiThreadedApplyLanguageModelTask;
  using ucam::fsttools::ApplyLMData;
  using namespace HifstConstants;
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n====================="
                         , "=====================" ) );
  // Detect here kenlm binary type
  // it's a bit ugly this way of initializing the correct kenlm handler
  lm::ngram::ModelType kenmt = ucam::util::detectkenlm (rg.getVectorString (
                                 kLmLoad, 0) );
  if (rg.get<std::string> (kHifstSemiring.c_str() ) ==
      kHifstSemiringStdArc )
    switch (kenmt) {
    case lm::ngram::PROBING:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::ProbingModel> >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::ProbingModel>  > >
        ( rg ) ) ();
      break;
    case lm::ngram::REST_PROBING:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::RestProbingModel>
        , lm::ngram::RestProbingModel >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::RestProbingModel>
        , lm::ngram::RestProbingModel  > > ( rg ) ) ();
      break;
    case lm::ngram::TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::TrieModel>, lm::ngram::TrieModel >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::TrieModel>, lm::ngram::TrieModel  > >
        ( rg ) ) ();
      break;
    case lm::ngram::QUANT_TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel  > >
        ( rg ) ) ();
      break;
    case lm::ngram::ARRAY_TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel  > >
        ( rg ) ) ();
      break;
    case lm::ngram::QUANT_ARRAY_TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel  > >
        ( rg ) ) ();
      break;
    }
  else if (rg.get<std::string> (kHifstSemiring) ==
           kHifstSemiringLexStdArc )
    switch (kenmt) {
    case lm::ngram::PROBING:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::ProbingModel>, lm::ngram::ProbingModel, fst::LexStdArc >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::ProbingModel>, lm::ngram::ProbingModel, fst::LexStdArc  > >
        ( rg ) ) ();
      break;
    case lm::ngram::REST_PROBING:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel, fst::LexStdArc >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::RestProbingModel>, lm::ngram::RestProbingModel , fst::LexStdArc > >
        ( rg ) ) ();
      break;
    case lm::ngram::TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::TrieModel>, lm::ngram::TrieModel, fst::LexStdArc >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::TrieModel>, lm::ngram::TrieModel, fst::LexStdArc  > >
        ( rg ) ) ();
      break;
    case lm::ngram::QUANT_TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel, fst::LexStdArc >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::QuantTrieModel>, lm::ngram::QuantTrieModel, fst::LexStdArc  > >
        ( rg ) ) ();
      break;
    case lm::ngram::ARRAY_TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel, fst::LexStdArc >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::ArrayTrieModel>, lm::ngram::ArrayTrieModel, fst::LexStdArc  > >
        ( rg ) ) ();
      break;
    case lm::ngram::QUANT_ARRAY_TRIE:
      ( Runner2<SingleThreadedApplyLanguageModelTask<ApplyLMData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel, fst::LexStdArc >
        , MultiThreadedApplyLanguageModelTask< ApplyLMData<lm::ngram::QuantArrayTrieModel>, lm::ngram::QuantArrayTrieModel, fst::LexStdArc  > >
        ( rg ) ) ();
      break;
    }
  else  {
    LERROR ("Semiring not supported (semiring => lexstdarc,stdarc)");
  }
  FORCELINFO ( argv[0] << " ends!" );
};

