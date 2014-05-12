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

/** \file tests/main.gtest.hifst.cpp
 * \brief Unit testing: main file.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include "googletesting.h"
#include "main.logger.hpp"
#include "main.custom_assert.hpp"

/**
 * \brief main function.
 * Note that Compiling this file will include all available tests into one single binary.
 */

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
