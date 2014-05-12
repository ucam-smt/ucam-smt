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

#ifndef BACKTRACE_HPP
#define BACKTRACE_HPP

/**
 * \file
 * \brief Adapted from http://charette.no-ip.com:81/programming/2010-01-25_Backtrace/
 *  Simple example of backtrace(), backtrace_symbols(), and __cxa_demangle()
 *  Remember to pass in the -rdynamic flag to GCC:
 * \date 11-10-2012
 * \author Gonzalo Iglesias
 */

#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
#include <cxxabi.h>
#endif

///Display backtrace of functions
inline int displayBacktrace ( void ) {
  const int size = 20;
  void *buffer[size];
  int ret;
  int idx;
  char **ptr;
  // get the void* pointers to functions; "ret" is the number of items returned
  ret = backtrace ( buffer, size );
  // now we want to convert those pointers to text
  ptr = backtrace_symbols ( buffer, ret );
  for ( idx = 0; idx < ret; idx ++ ) {
    // if using C++, you can look up abi::__cxa_demangle( ... ) in #include <cxxabi.h>
    printf ( "%d: %s\n", idx, ptr[ idx ] );
  }
  free ( ptr );
  ///\todo: Function names are more or less readable, but we could further demangle them using something like the following piece of code:
  /*
  #ifdef __cplusplus
  // here is an example of C++ demangling
  int status = -1;
  char *demangledName = abi::__cxa_demangle( "_Z16displayBacktracev", NULL, NULL, &status );
  if ( status == 0 )
    {
      printf( "_Z16displayBacktracev demangles to:  %s\n", demangledName );
    }
  free( demangledName );
  #endif

  */
  return ret;
}

#endif // BACKTRACE_HPP
