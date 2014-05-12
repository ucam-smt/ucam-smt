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
 * \brief Logger implementation -- init method and macros around actual Boost Logger
 * \date 2012
 * \author Gonzalo Iglesias
 */

#ifndef LOGGER_BOOST_LOG_HPP
#define LOGGER_BOOST_LOG_HPP

#include <iostream>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>

#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>

namespace ucam {
namespace util {

namespace logging = boost::log;

namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

/**
 * \brief Inits logger, parses param options checking for --logger.verbose
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
  logging::add_console_log (
    std::clog,
    keywords::format = "[%TimeStamp%] %_%"
  );
};

}
} // end namespaces

#ifdef PRINTINFO
#define LINFO(msg)       if (!silent::get()) BOOST_LOG_TRIVIAL(info) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".INF:" << msg ;
#define FORCELINFO(msg)  BOOST_LOG_TRIVIAL(info) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__) << ".INF:" << msg ;
#else
#define LINFO(msg)
#define FORCELINFO(msg)
#endif

#ifdef PRINTDEBUG3
#ifndef PRINTDEBUG2
#define PRINTDEBUG2
#endif
#define LDEBUG3(msg)       BOOST_LOG_TRIVIAL(debug) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG3:" << msg ;
#else
#define LDEBUG3(msg)
#endif

#ifdef PRINTDEBUG2
#ifndef PRINTDEBUG1
#define PRINTDEBUG1
#endif
#define LDEBUG2(msg)       BOOST_LOG_TRIVIAL(debug) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG2:" << msg ;
#else
#define LDEBUG2(msg)
#endif

#ifdef PRINTDEBUG1
#ifndef PRINTDEBUG
#define PRINTDEBUG
#endif
#define LDEBUG1(msg)       BOOST_LOG_TRIVIAL(debug) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG1:" << msg ;
#define LDEBUG(msg)       BOOST_LOG_TRIVIAL(debug) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".DBG1:" << msg ;
#else
#define LDEBUG(msg)
#endif

#ifdef PRINTDEBUG
#define LDBG_EXECUTE(order) {order;}
#else
#define LDBG_EXECUTE(order)
#endif

#ifdef PRINTERROR
#define LERROR(msg)       BOOST_LOG_TRIVIAL(error) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".ERR:" << msg ;
#else
#define LERROR(msg)
#endif

#ifdef PRINTWARNING
#define LWARN(msg)       BOOST_LOG_TRIVIAL(warning) << ::ucam::util::filteredHeader(::ucam::util::detailed?__PRETTY_FUNCTION__:__func__)<< ".WRN:" << msg ;
#else
#define LWARN(msg)
#endif

#endif
