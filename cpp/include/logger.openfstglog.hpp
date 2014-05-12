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
 * \brief Logger implementation -- init method and macros around actual OpenFST logger -- requires including openfst libraries first
 * \date 2012
 * \author Gonzalo Iglesias
 */

#ifndef LOGGER_OPENFSTGLOG_HPP
#define LOGGER_OPENFSTGLOG_HPP

namespace ucam {
namespace util {

/**
 * \brief Inits logger, parses param options checking for --logger.verbose.
 * \param argc: Standard main argc parameter
 * \param argv: Standard main argv parameter
 */
inline void initLogger ( int argc, const char *argv[] ) {
  bool verbose = false;
  const std::string lv = "--" + HifstConstants::kLoggerVerbose;
  for ( uint k = 0; k < argc; ++k ) {
    if ( !strcmp ( argv[k], lv.c_str() ) ) {
      verbose = true;
      break;
    }
  }
  silent::set ( !verbose );
};

}
}

#ifdef PRINTINFO
#define LINFO(msg)       if (!::ucam::util::silent::get()) TRACER << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".INF:" << msg  ;
#define FORCELINFO(msg)  TRACER << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__) << ".INF:" << msg  ;
#else
#define LINFO(msg)
#define FORCELINFO(msg)
#endif

#ifdef PRINTDEBUG3
#ifndef PRINTDEBUG2
#define PRINTDEBUG2
#endif
#define LDEBUG3(msg)       TRACER << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG3:" << msg  ;
#else
#define LDEBUG3(msg)
#endif

#ifdef PRINTDEBUG2
#ifndef PRINTDEBUG1
#define PRINTDEBUG1
#endif
#define LDEBUG2(msg)       TRACER << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG2:" << msg  ;
#else
#define LDEBUG2(msg)
#endif

#ifdef PRINTDEBUG1
#ifndef PRINTDEBUG
#define PRINTDEBUG
#endif
#define LDEBUG1(msg)       TRACER << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG1:" << msg  ;
#define LDEBUG(msg)       TRACER << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG1:" << msg  ;
#else
#define LDEBUG(msg)
#endif

#ifdef PRINTDEBUG
#define LDBG_EXECUTE(order) {order;}
#else
#define LDBG_EXECUTE(order)
#endif

#ifdef PRINTERROR
#define LERROR(msg)       TRACER  << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".ERR:" << msg  ;
#else
#define LERROR(msg)
#endif

#ifdef PRINTWARNING
#define LWARN(msg)       TRACER  << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".WRN:" << msg  ;
#else
#define LWARN(msg)
#endif

#endif
