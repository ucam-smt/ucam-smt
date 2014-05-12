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

/** \file include/addresshandler.hpp
 * \brief Handles simple wildcard expansion for strings.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef ADDRESSHANDLER_HPP
#define ADDRESSHANDLER_HPP

namespace ucam {
namespace util {
/**
 *\brief class that expands a wildcard into its actual value.
 * This is useful e.g. for filenames ranging several sentences
 * \remark. This class can be trivially  extended to deal automatically with key-dependent wildcards, e.g. %%range%%, etc.
 */

template<typename T = uint>
class PatternAddress {
 private:
  ///String with wildcards
  std::string address_;
  ///Wildcard used
  std::string wildcard_;

 public:

  /**
   *\brief Constructor
   * \param address string containing wildcards
   * \param wildcard Wildcard that will be parsed. By default, ?
   */

  PatternAddress ( const std::string& address,
                   const std::string& wildcard = "?" ) :
    address_ ( address ),
    wildcard_ ( wildcard ) {
  };

  /**
   *\brief Expands string and returns
   *\param idx Actual index to expand the wildcard with.
   */

  inline const std::string get ( T idx ) {
    std::string tmpaddress = address_;
    LDEBUG ( "Parsing [" << tmpaddress << "]=>" << idx );
    parse ( idx, tmpaddress );
    return tmpaddress;
  }

  /**
   *\brief Expands string and returns
   *\param idx Actual index to expand the wildcard with.
   *\returns expanded string.
   */

  inline const std::string operator() ( T idx ) {
    return get ( idx );
  }

  /**
   *\brief Returns original string, without replacing any wildcard.
   *\returns Original string.
   */

  inline const std::string operator() () {
    return address_;
  }

 private:
  /**
   *\brief Expands wildcard rewriting the with actual values.
   * \param idx Actual index to expand wildcard with.
   */

  inline void parse ( T idx, std::string& tmpaddress ) {
    find_and_replace ( tmpaddress, wildcard_, toString<T> ( idx ) );
  };

};

typedef PatternAddress<uint> IntegerPatternAddress;

}
} // end namespaces

#endif
