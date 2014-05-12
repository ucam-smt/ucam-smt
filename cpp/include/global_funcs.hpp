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

/** \file include/global_funcs.hpp
 * \brief General functions
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef GLOBAL_FUNCTIONS_HPP
#define GLOBAL_FUNCTIONS_HPP

/** \file global_funcs.hpp
 * General convenience functions and classes
 */

namespace ucam {
namespace util {
/**
 * \brief Converts an arbitrary type to string
 * Converts to string integers, floats, doubles
 * Quits execution if conversion has failed.
 * \param x Templated variable to be converted.
 * \param pr Precision (only useful for float/double)
 */
template <typename T>
inline std::string toString ( const T& x, uint pr = 2 ) {
  std::stringstream aux;
  aux << std::fixed << std::setprecision ( pr ) << x;
  std::string ss;
  aux >> ss;
  USER_CHECK ( !aux.fail(), "Converting Number to string failed" );
  return ss;
};

/**
 * \brief Converts an arbitrary type to string
 * Converts to string integers, floats, doubles
 * Quits execution if conversion has failed.
 * \param x Templated variable to be converted.
 * \param ss
 * \param pr Precision (only useful for float/double)
 */
template <typename T>
inline void toString ( const T& x, std::string& ss, uint pr = 2 ) {
  std::stringstream aux;
  if ( pr != std::numeric_limits<T>::max() ) aux << std::fixed <<
        std::setprecision ( pr ) << x;
  else aux << x ;
  aux >> ss;
  USER_CHECK ( !aux.fail(), "Converting Number to string failed" );
};

/**
 * \brief Converts a string to an arbitrary number
 * Converts strings to a number.
 * Quits execution if conversion has failed.
 * \param x Templated variable to be converted.
 */
template <typename T>
inline T toNumber ( const std::string& x ) {
  std::stringstream aux;
  aux << x;
  T ss;
  aux >> ss;
  USER_CHECK ( !aux.fail() && aux.eof(), "Converting string to number failed" );
  return ss;
};

///Convenience function to find out whether a needle exists in a text.
inline bool exists ( const std::string& source, const std::string& needle ) {
  return ( source.find ( needle ) != std::string::npos );
};

///Convenience function to find a needle and replace it.
/// \todo There might exist a boost utility for find_and_replace
inline void find_and_replace ( std::string& haystack, const std::string& needle,
                               const std::string& replace ) {
  std::size_t j = 0;
  for ( ; ( j = haystack.find ( needle ) ) != std::string::npos ; ) {
    haystack.replace ( j, needle.length(), replace );
  }
};

///Convenience function that determines whether a string ends with another string.
///\todo Possibly use a boost string function instead?
inline bool ends_with ( std::string const& haystack,
                        std::string const& needle ) {
  if ( haystack.length() >= needle.length() )
    return ( 0 == haystack.compare ( haystack.length() - needle.length(),
                                     needle.length(), needle ) );
  return false;
};

///Convenience function that counts the number of times a needle appears.
inline uint count_needles ( const std::string& haystack, const char needle,
                            std::size_t start, std::size_t end ) {
  uint count = 0;
  for ( const char *c = haystack.c_str() + start; c < haystack.c_str() + end;
        ++c )
    if ( *c == needle ) ++count;
  return count;
}

///Trims spaces at the edges (no spaces) and also between words (only one space)
inline void trim_spaces ( const std::string& input, std::string *output ) {
  boost::regex pattern ( "\\s+",
                         boost::regex_constants::icase | boost::regex_constants::perl );
  std::string replace ( " " );
  *output = boost::regex_replace ( input, pattern, replace );
  boost::algorithm::trim ( *output );
};

///Trims decimal zeros at the end of a floating number represented as a string.
///Use sparingly -- in general prefer float variables. ;-)

inline void trim_trailing_zeros ( std::string& snumber ) {
  uint k;
  for (k = snumber.size() - 1; k > 0 && snumber[k] == '0'; --k);
  if (k == snumber.size() - 1 || !k) return;
  uint j = k;
  for (; j > 0 && snumber[j] != '.'; --j);
  if (j != k) snumber = snumber.substr (0, k + 1);
  else if (j > 0) snumber = snumber.substr (0, k);
}

/**
 * \brief Checks whether the sentence is in format  ^\\d+( \\d+)*$
 * \param s: string to validate
 * \return true if validated, false otherwise
 */
inline bool validate_source_sentence ( const std::string& s ) {
  static const boost::regex e ( "^\\d+( \\d+)*$" );
  return boost::regex_match ( s, e );
}

///Generates time stamp.
inline std::string getTimestamp ( void ) {
  time_t ltime;
  time ( &ltime );
  char time_buffer[128];
  sprintf ( time_buffer, "%s", ctime ( &ltime ) );
  time_buffer[strlen ( time_buffer ) - 1] = 0;
  std::string times;
  times.append ( time_buffer );
  return times;
};

/// Retrieves directory name for a filename path.
///\todo: use boost::filesystem?
inline bool DirName ( std::string& dirname, const std::string& filename ) {
  std::string::size_type loc = filename.find_last_of ( "/", filename.size() );
  if ( loc == std::string::npos ) return false;
  dirname.append ( filename, 0, loc );
  return true;
};

///Checks wether a file exists or not.
///\todo: use boost::filesystem?
inline bool fileExists ( const std::string& fileName ) {
  std::fstream fin;
  fin.open ( fileName.c_str(), std::ios::in );
  if ( fin.is_open() ) {
    fin.close();
    return true;
  }
  fin.close();
  return false;
};

///Implements dot product
inline float dotproduct ( std::vector<float>& v1, std::vector<float>& v2 ) {
  std::size_t lowertop = v1.size() > v2.size() ? v2.size() : v1.size();
  float result = 0.0f;
  for ( std::size_t k = 0; k < lowertop; ++k )
    result += v1[k] * v2[k];
  return result;
}

///Functor implementing comparison for hashes when used with basic_string<uint>
class hasheqvecuint {
 public:
  ///Implements comparison
  bool operator() ( std::basic_string<uint> const& v1,
                    std::basic_string<uint> const& v2 ) const {
    return v1 == v2;
  }
};

///Functor implementing hash function for hashes with basic_string<uint>
class hashfvecuint : public
  std::unary_function<std::basic_string<uint>, std::size_t> {
 public:
  ///Implements hash function
  std::size_t operator() ( std::basic_string<uint> const& v ) const {
    std::size_t res = 0;
    for ( unsigned int i = 0; i < v.size();
          i++ )  res += ( std::size_t ) pow ( ( double ) v[i],
                          double ( ( ( i ) % 6 ) + 1 ) );
    return res % HASH_MODULE;
  }
};

}
} // end namespaces

#endif
