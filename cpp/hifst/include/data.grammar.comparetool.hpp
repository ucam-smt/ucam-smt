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

#ifndef DATA_GRAMMAR_COMPARETOOL_HPP
#define DATA_GRAMMAR_COMPARETOOL_HPP

/**
 * \file
 * \brief Contains structures and classes for GrammarData
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 *\brief Struct containing rule positions and offsets.
 *
 * Positions are needed to mark the place where the rule has to be searched for (e.g. source phrase).
 * Offsets are needed to indicate where the rule actually starts.
 * \remark Important note: There is a current limitation with this structure. If the number of positions  is greater than 4294967295 (~4096MB) expect your tool to go haywire!
 */

struct posindex {
  /// position
  std::size_t p;
  /// offset
  short o;
  /// absolute index
  std::size_t order;
};

/**
 *\brief Class that provides basic string comparison between two const char *.
 *
 */

class CompareTool {
 public:
  virtual __always_inline int compare ( const char *s1, const char *s2 ) {
    return strcmp ( s1, s2 );
  };
  virtual __always_inline int ncompare ( const char *s1, const char *s2,
                                         uint n ) {
    return strncmp ( s1, s2, n );
  }

};

/**
 *\brief Functor Class that provides comparison accross the posindex structure.
 * This is typically used e.g. with a priority queue. It can use CompareTool or any inherited class.
 *
 */

class PosIndexCompare {
 private:
  std::string *s_;
  CompareTool *ct_;

 public:
  inline PosIndexCompare ( std::string *c, CompareTool *myct ) : s_ ( c ),
    ct_ ( myct ) {};
  inline bool operator() ( const posindex& lhs, const posindex& rhs ) const  {
    const char *nh = s_->c_str();
    if ( ct_->compare ( nh + lhs.p,
                        nh + rhs.p ) <= 0 ) return
                            true; /// e.g. priority queue will decide how to reorder according to what is returned here.
    return false;
  };
};

/**
 *\brief Class that provides "pattern" comparison between two const char *.
 * The "patterns" are an abstraction of any non-terminal A-Z. So for instance, consider non_terminals A and Z.
 * A rule with source 3_A_5 and another one with source 3_Z_5 are equivalent and need to be listed together
 * (with any other equivalent sources).
 * This class can be used with PosIndexCompare as it inherits from CompareTool.
 */

class PatternCompareTool: public CompareTool {
 public:
  virtual __always_inline int compare ( const char *s1,
                                        const char
                                        *s2 ) { //IMPORTANT: s1 is portion (source side of rule) of file, s2 is portion of file. DO NOT use for pattern search!
    for ( ; *s1 == *s2 || ( *s1 >= 'A' && *s1 <= 'Z' && *s2 >= 'A'
                            && *s2 <= 'Z' ); ++s1, ++s2 ) {
      if ( *s1 == ' ' ) return
          0; //we don't really care about the order of translations
      if ( *s1 == 0 ) return 0;
      if ( *s2 >= 'A' && *s2 <= 'Z' ) while ( * ( s2 + 1 ) != '_'
                                                && * ( s2 + 1 ) != ' ' && * ( s2 + 1 ) != '\0' ) s2++; // jump indices.
      if ( *s1 >= 'A' && *s1 <= 'Z' ) while ( * ( s1 + 1 ) != '_'
                                                && * ( s1 + 1 ) != ' ' && * ( s1 + 1 ) != '\0' ) s1++; // jump indices.
    }
    return * ( const unsigned char * ) s1 - * ( const unsigned char * ) s2;
  };

  //IMPORTANT: s1 is pattern and MUST use X for generic non-terminals! ended with space, s2 is portion of file, n is size of pattern
  virtual __always_inline int ncompare ( const char *s1, const char *s2,
                                         uint n ) {
    if ( n == 0 ) return
        0;                                                    /// Nothing to compare?  Return zero.
    while ( n-- > 0 && ( *s1 == *s2 || ( *s1 == 'X' && *s2 >= 'A'
                                         && *s2 <= 'Z' ) ) ) { // Loop, comparing bytes.
      if ( n == 0 || *s1 == '\0' ) return 0;
      s1++; //it's a pattern, no indices after generic non-terminal, just increment
      if ( *s2 >= 'A' && *s2 <= 'Z' ) while ( * ( s2 + 1 ) != '_'
                                                && * ( s2 + 1 ) != ' ' ) s2++; // jump  any indices.
      s2++;
    }
    unsigned char uc1, uc2;
    uc1 = ( * ( unsigned char * ) s1 );
    uc2 = ( * ( unsigned char * ) s2 );
    return uc1 - uc2;
  }
};

}
} // end namespaces

#endif
