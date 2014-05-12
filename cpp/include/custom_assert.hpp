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

/** \file include/custom_assert.hpp
 * \brief Provides smarter assert methods.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef CUSTOM_ASSERT
#define CUSTOM_ASSERT

#include <fstream>
#include <iostream>

#ifndef OSR
#include "backtrace.hpp"
#endif

/**
 *  \def USER_CHECK(exp,comment)
 *  \brief Tests whether exp is true. If not, comment is printed and program ends.
 */
#define USER_CHECK_CONTINUE(exp,comment) (bool)( (exp) || (check (#exp,#comment, __FILE__, __PRETTY_FUNCTION__,__LINE__,false), 0) )

#ifdef USER_CHECK_DEBUG
extern bool user_check_ok;
#define USER_CHECK(exp,comment) (bool)( (exp) || (check (#exp,#comment, __FILE__, __PRETTY_FUNCTION__,__LINE__,false), 0) )
#else
#define USER_CHECK(exp,comment) (bool)( (exp) || (check (#exp,#comment, __FILE__, __PRETTY_FUNCTION__,__LINE__,true), 0) )
#endif

/**
 *\brief Function that reports as many details as possible if the assertion USER_CHECK has failed.
 * Combined with USER_CHECK, it reports file, function/class method, and file line. In addition, execution is ended unless USER_CHECK_DEBUG defined, and a reason is reported.
 * the value returned by check can also be used to decide subsequent actions in USER_CHECK_DEBUG mode (e.g. return inmediately).
 * check function also provides backtrace information to help the user.
 */

inline bool check ( const char * expression, const char * message,
                    const char *filename, const char *function, uint line , bool abort = true) {
  std::cerr << "==========================================================" <<
            std::endl;
  std::cerr << "User check failure at " << filename << ",function=" << function <<
            ",line=" <<  line << std::endl;
  std::cerr << "Reason: " << message << std::endl;
  std::cerr << "Test: " <<  expression  << std::endl;
#ifndef OSR
  displayBacktrace();
#endif
  if (abort) {
    std::cerr << "Aborting!" << std::endl;
    std::cerr << "==========================================================" <<
              std::endl;
    exit ( EXIT_FAILURE );
  }
  std::cerr << "==========================================================" <<
            std::endl;
#ifdef USER_CHECK_DEBUG
  return user_check_ok = false; //we can now get rid of this
#endif
  return false;
};

#endif
