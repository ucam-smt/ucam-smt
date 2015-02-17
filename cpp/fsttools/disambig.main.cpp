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
 * \brief Main file for disambig tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <main.disambig.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>

template<class ArcT >
void run (ucam::util::RegistryPO const& rg) {
  using ucam::fsttools::DisambigData;
  using ucam::fsttools::DisambigTask;
  using ucam::fsttools::ReadFstTask;
  using ucam::fsttools::WriteFstTask;
  using ucam::fsttools::LoadUnimapTask;
  using ucam::fsttools::LoadLanguageModelTask;
  DisambigData data;
  //Our pipeline is defined by the following tasks:
  //Load Recasing language model
  boost::scoped_ptr< LoadLanguageModelTask<DisambigData > > mytask (
    new LoadLanguageModelTask<DisambigData> ( rg ,
        HifstConstants::kRecaserLmLoad , HifstConstants::kRecaserLmFeatureweight ) );
  mytask->appendTask
  //Load unigram transduction model
  ( new LoadUnimapTask< DisambigData, ArcT > ( rg  ,
      HifstConstants::kRecaserUnimapLoad, HifstConstants::kRecaserLmLoad ) )
  //Read input lattice
  ( new ReadFstTask<DisambigData, ArcT > ( rg  ,
      HifstConstants::kRecaserInput ) )
  //Apply both models and prune
  ( new DisambigTask<DisambigData, ArcT > ( rg  ,
      HifstConstants::kRecaserInput , HifstConstants::kRecaserOutput ,
      HifstConstants::kRecaserLmLoad , HifstConstants::kRecaserUnimapLoad ) )
  //Write output lattice
  ( new WriteFstTask<DisambigData, ArcT > ( rg  ,
      HifstConstants::kRecaserOutput ) )
  ;
  //For a given range of lattices, proceed to run the task
  for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg,
                                    HifstConstants::kRangeOne ) )
        ; !ir->done()
        ; ir->next() ) {
    data.sidx = ir->get();
    mytask->chainrun ( data );
  }
}

/**
 * \brief Main function for disambig tool. Single-threaded implementation only.
 * Applies a unigram transduction model and a language model to account for the context.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 */

int main ( int argc, const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "=====================" ) )  ;
  if (rg.get<std::string> (HifstConstants::kHifstSemiring) ==
      HifstConstants::kHifstSemiringStdArc) run<fst::StdArc> (rg);
  else if (rg.get<std::string> (HifstConstants::kHifstSemiring) ==
           HifstConstants::kHifstSemiringLexStdArc) run<fst::LexStdArc>
    (rg);
  else {
    LERROR ("Unknown semiring type!");
  }
  FORCELINFO ( argv[0] << " ends!" );
};

