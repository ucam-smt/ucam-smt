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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#define PRINTLOG
#define PRINTERROR
#define PRINTWARNING
//#define PRINTDEBUG
#define PRINTINFO

namespace HifstConstants {
const std::string kLoggerVerbose = "logger.verbose";
}

namespace ucam {
namespace util {

std::string getTimestamp ( void );

///Provides methods to set and get silent logging mode
class silent {
 private:
  ///Actual variable defining the logging mode
  static bool silent_;
 public:
  ///Return mode
  inline static bool get () {
    return silent_;
  };
  ///Set silent mode or viceversa
  inline static void set ( bool silent ) {
    silent_ = silent;
  };
};

///This function is meant to filter  __PRETTY_FUNCTION__ and attempts to simplify it. With the templating it can get a little nasty to show in the logs.
inline std::string filteredHeader ( const std::string& a )  {
  size_t pos = a.find_last_of ( '(' );
  size_t pos2 = a.find_last_of ( ' ', pos );
  return a.substr ( pos2 + 1, pos - pos2 - 1 );
};
}
}

//Choose your favourite logger.
//OpenFST has a vanilla GLOG integrated --note that real GLOG has compatibility issues with openfst 1.3.1+
//BOOSTLOG is great library, candidate to boost -- but not officially part of the release yet.
// should be easy to use it, although it hasn't been tested for a while...
#ifdef USE_BOOSTLOG
//Uses boost log -- It is candidate to the boost library, but not an official release yet, as of late 2012.
#include "logger.boost_log.hpp"
#else
//By default will take OpenFST's implementation! Need to include openfst headers before logger.hpp
#include <fst/fstlib.h>
#define TRACER  LogMessage(ucam::util::getTimestamp()).stream()
#include "logger.openfstglog.hpp"
#endif

//When in the future Google OpenFST is easy to compile with Google Log, we can cover it easily here.

#endif
