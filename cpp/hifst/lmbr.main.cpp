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
#include <main.lmbr.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main-run.lmbr.hpp>

/**
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks     Main function. Runs alilats2splats tool. See main-run.alilats2splats.hpp
 */

int
main ( int argc, const char *argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n====================="
                         , "=====================" ) );
  ( ucam::util::Runner2<ucam::lmbr::SingleThreadedLmbrTask<>
    , ucam::lmbr::MultiThreadedLmbrTask<>  > ( rg ) ) ();
  FORCELINFO ( argv[0] << " ends!" );
  return 0;
}
