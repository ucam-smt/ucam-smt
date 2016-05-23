//Copyright (c) 2012, University of Cambridge
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met://
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of Cambridge nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PARAMS_H_
#define PARAMS_H_

#include <limits.h>

namespace fst {

template<typename T, typename Collection>
struct AE {
  static void AddElement (Collection&, unsigned int, T);
};

template<typename T>
struct AE<T, std::vector<T> > {
  static void AddElement (std::vector<T>& params, unsigned int index, T param) {
    if (params.size() == index) {
      params.push_back (param);
    } else if (params.size() < index) {
      params.resize (index + 1, 0.0);
      params[index] = param;
    }
  }
};

template<typename T>
struct AE<T, std::map<unsigned int, T> > {
  static void AddElement (std::map<unsigned int, T>& params , unsigned int index
                          , T param) {
    params[index] = param;
  }
};

template<typename T, typename Collection >
Collection ParseParamString (const std::string& stringparams) {
  Collection result;
  std::stringstream strm (std::stringstream::in | std::stringstream::out);
  strm << stringparams << noskipws;
  char separator;
  unsigned int index = 0;
  while (strm.good() ) {
    T w;
    if (strm.peek() == '@') {
      w = 1.0;
      strm >> separator;
    } else {
      strm >> w;
      if (strm.eof() ) {
        separator = ' ';
      } else {
        strm >> separator;
      }
    }
    if (separator == '@') {
      strm >> index;
      if (!strm.eof() ) {
        strm >> separator;
      }
    }
    AE<T, Collection>::AddElement (result, index, w);
    ++index;
  }
  if (strm.fail() || strm.bad() ) {
    std::cerr << "ERROR: Unable to parse params: " << stringparams << std::endl;
    exit (1);
  }
  return result;
}

template<typename T>
struct ParamsInit {
  std::vector<T> params;

  ParamsInit() {
    std::string stringparams;
    char *paramsfile = getenv ("PARAMS_FILE");
    if (paramsfile) {
      ifstream ifs (paramsfile);
      if (!ifs.good() ) {
        std::cerr << "ERROR: unable to open file " << paramsfile << '\n';
        exit (1);
      }
      getline (ifs, stringparams);
    } else {
      char * pParams = getenv ("PARAMS");
      if (!pParams) {
        if (FLAGS_v > 0) {
          std::cerr
              << "Warning: cannot find parameter vector. Defaulting to flat parameters\n";
        }
        return;
      }
      stringparams = pParams;
    }
    params = ParseParamString<T, std::vector<T> > (stringparams);
    if (FLAGS_v > 0) {
      std::cerr << "Setting params to: ";
      for (typename std::vector<T>::const_iterator it = params.begin();
           it != params.end(); ++it) {
        std::cerr << *it << ", ";
      }
      std::cerr << std::endl;
    }
  }

};

}

#endif /* PARAMS_H_ */
