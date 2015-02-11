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

#ifndef PARAMS_HPP_
#define PARAMS_HPP_

/**
 * \file
 * \brief Convenience functions to parse parameters from a string
 * \remark This file is used by our dynamic libraries. 
 * functionality.
 * \date 2010-2012
 * \author Rory Waite
 */

// To avoid bloating, functions in this file do not use need to 
// use logger or any boost library, etc.
#ifndef LINFO
#define LINFO(x) std::cerr << "INFO:: " << x << std::endl;
#endif
#ifndef LERROR
#define LERROR(x) std::cerr << "ERROR:: " << x << std::endl;
#endif
#ifndef LWARN
#define LWARN(x) std::cerr << "WARNING:: " << x << std::endl;
#endif

namespace ucam {
namespace util {

/**
 * \brief Function to parse string of parameters, e.g. separated by commas
 */

template<typename T>
inline std::vector<T> ParseParamString ( const std::string& stringparams ,
    size_t pos = 0 ) {
  std::vector<T> result;
  std::stringstream strm ( std::stringstream::in | std::stringstream::out );
  if ( pos ) strm << stringparams.substr ( pos ) << std::noskipws;
  else strm << stringparams << std::noskipws;
  char separator;
  while ( strm.good() ) {
    if ( result.size() > 0 ) {
      strm >> separator;
    }
    T w;
    strm >> w;
    result.push_back ( w );
  }
  if ( strm.fail() || strm.bad() || strm.fail() ) {
    LERROR("Unable to parse params : " << stringparams.substr ( pos ) );
    for ( unsigned k = 0; k < result.size(); ++k )
      std::cerr << result[k] << std::endl;
    exit ( EXIT_FAILURE );
  }
  return result;
}

///Version 2, passing output by reference...
template<typename T>
inline void ParseParamString ( const std::string& stringparams ,
                               std::vector<T>& params, size_t pos = 0 ) {
  std::stringstream strm ( std::stringstream::in | std::stringstream::out );
  if ( pos ) strm << stringparams.substr ( pos ) << std::noskipws;
  else strm << stringparams << std::noskipws;
  char separator;
  while ( strm.good() ) {
    if ( params.size() > 0 ) {
      strm >> separator;
    }
    T w;
    strm >> w;
    params.push_back ( w );
  }
  if ( strm.fail() || strm.bad() || strm.fail() ) {
    LERROR("Unable to parse params : " << stringparams.substr ( pos ) );
    for ( unsigned k = 0; k < params.size(); ++k )
      std::cerr << params[k] << std::endl;
    exit ( EXIT_FAILURE );
  }
}

/**
 * \brief Write parameter vector to a file, with comma separators
 */
inline void WriteParamFile(const std::string& filename, std::vector<float> params_) {
  std::ofstream ofs ( filename.c_str() );
  if ( !ofs.good() ) {
    LERROR("Can't write to " << filename);
    exit ( EXIT_FAILURE );
  }
  LINFO("Writing final Lambda to " << filename );
  float ev = params_.back();
  params_.pop_back();
  for (size_t i=0; i<params_.size(); i++) {
    ofs << params_[i] << ",";
  }
  ofs << ev << std::endl;
  ofs.close();
}

/**
 * \brief Initializes a set of parameters from environment variables PARAMS_FILE or PARAMS
 */
template<typename T>
struct ParamsInit {
  std::vector<T> params;

  ParamsInit() {
    std::string stringparams;
    char *paramsfile = getenv ( "TUPLEARC_WEIGHT_VECTOR_FILE" );
    if ( paramsfile ) {
      std::ifstream ifs ( paramsfile );
      if ( !ifs.good() ) {
	LERROR( "Unable to open file " << paramsfile );
        exit ( EXIT_FAILURE);
      }
      getline ( ifs, stringparams );
    } else {
      char * pParams = getenv ( "TUPLEARC_WEIGHT_VECTOR" );
      if ( !pParams ) {
        LWARN("Cannot find parameter vector. Defaulting to flat parameters");
        return;
      }
      stringparams = pParams;
    }
    params = ParseParamString<T> ( stringparams );
#ifdef PRINTDEBUG
    std::stringstream ss;
    for ( typename std::vector<T>::const_iterator it = params.begin();
          it != params.end(); ++it ) {
      ss << *it << ", ";
    }
    std::cerr << "Setting params to: " << ss.str() << endl;
#endif
  }

};

}
} // end namespaces
#endif /* PARAMS_HPP_ */
