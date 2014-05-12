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

#ifndef WORDMAPPER_HPP
#define WORDMAPPER_HPP

/** \file
 *  \brief class WordMapper
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace util {

/**
 * \brief comparison functor for queue sorting.
 *
 */

class PQwmapcompare {
 private:
  std::string *pdata_;
  unsigned *pt_;

 public:
  /// constructor
  inline PQwmapcompare ( std::string *pd, unsigned *pt ) : pdata_ ( pd ),
    pt_ ( pt ) {};
  /// string comparison between two positions i1 and i2.
  inline bool operator() ( const unsigned& i1, const unsigned& i2 ) const  {
    return strcmp ( pdata_->c_str() + pt_[i1], pdata_->c_str() + pt_[i2] ) >= 0;
  };
};

/**
 *\brief Loads efficiently a wordmap file and provides methods to map word-to-integer or integer-to-word.
 * To avoid memory footprint issues, hashing the wordmap entries is avoided.
 *
 * \remark For both directions, the file always takes the following format:
 * what          59
 * report        60
 * council       61
 *
 * Also assumes:
 * - Bijective relationship (word <-> integer id)
 * - Sorted by id and no id missing (if 61 exists, 59 and 60 must exist in the file and appear in previous lines...
 * - First index is 0.
 * \remark OOV ids are also generated, if the word does not exist in the file.
 */

class WordMapper {

 private:
  /// To activate reverse search (mapping string to integer)
  bool reverse_;
  ///Indices
  unsigned *pt_;
  ///Sorted indices
  unsigned *ptr_;
  ///Number of words
  unsigned size_;
  ///Actual words stored here.
  std::string data_;

  ///\todo check and verify oovid
  unsigned oovid_;
  unordered_map<std::size_t, std::string>
  oovwmap_;  // pass this one to target and we will effectively have oov passthru.
  unordered_map<std::string, std::size_t> oovrwmap_;

 public:
  /**
   * \brief Constructor
   *
   * \remark Loads wordmap file. If mapping is to be performed from string-to-int, there is an additional id sorting step.
   * \param wordmapfile:  Wordmap file to load.
   * \param reverse: Perform string-to-integer (false) or integer-to-string(true).
   */
  WordMapper ( const std::string& wordmapfile, bool reverse = false ) :
    size_ ( 0 ),
    pt_ ( NULL ),
    reverse_ ( reverse ) {
    if ( wordmapfile == "" ) {
      LINFO ( "No word/integer map file!" );
      return;
    }
    FORCELINFO ( "Loading word mapper " << wordmapfile );
    iszfstream aux ( wordmapfile );
    load ( aux );
  };

  WordMapper ( iszfstream& wordmapstream, bool reverse = false ) :
    size_ ( 0 ),
    pt_ ( NULL ),
    reverse_ ( reverse ) {
    load ( wordmapstream );
  }

  /**
   * \brief Perform search. Both directions allowed (int to string or string to int).
   * \param is: input string
   * \param os: output string
   * \param reverse: if true, triggers reverse search (string to int).
   */

  inline void operator () ( const std::string& is, std::string *os,
                            bool reverse = false ) {
    LDEBUG ("Searching " << is);
    reverse ? mapstr2i ( is, os ) : mapi2str ( is, os );
    boost::algorithm::trim ( *os );
    return;
  }

  /**
   * \brief Quick hack to get what is needed for lm.
   * \remark Note: assumes only 1 number in the string, searches and returns unsigned.
   * if not found, returns max unsigned value.
   */
  inline unsigned operator () ( const std::string& is) {
    int result = this->bs ( data_, is + "\n" , 0, size_ - 1 );
    if ( result < 0 ) return std::numeric_limits<unsigned>::max();
    return ptr_[result];
  };

  ///Destructor
  ~WordMapper() {
    if ( pt_ == NULL ) return;
    delete [] pt_;
    if ( reverse_ ) delete [] ptr_;
  }

  ///Return oovwmap.
  inline unordered_map<std::size_t, std::string>& get_oovwmap() {
    return oovwmap_;
  };

  //Reset oovwmap with an external hash
  inline void set_oovwmap ( unordered_map<std::size_t, std::string>& oovmap ) {
    oovwmap_ = oovmap;
  };
  ///Resets oovid to lowest value
  inline void reset_oov_id() {
    oovid_ = OOVID;
  };

  //Returns actual oovid_...
  inline std::size_t get_oov_id() {
    return oovid_;
  };

 private:
  ///Loads wordmap from [file] stream
  void load ( iszfstream& aux ) {
    std::string line;
    while ( getline ( aux, line ) ) {
      size_++;
      std::stringstream x ( line );
      std::string aux;
      x >> aux;
      data_ += aux + "\n";
      unsigned s;
      x >> s;   // Integer ids discarded, but must agree with file position.
      if (s != size_ - 1 ) {
        LERROR ("Wrong wordmap file format.  Wordmap with sequential ids is required! =>"
                << aux << "<->" << s);
        exit (EXIT_FAILURE);
      }
    }
    aux.close();
    LINFO ( "number of lines: " << size_ );
    pt_ = new unsigned[size_];
    unsigned ci = 0;
    pt_[ci++] = 0;
    for ( unsigned k = 0; k < data_.size(); ++k ) {
      if ( data_[k] == '\n' && ci < size_ ) {
        pt_[ci++] = k + 1;
      }
    }
    if ( !reverse_ ) return;
    LINFO ( "generating sorted indices for reverse mapping of " << size_ <<
            " elements" );
    std::priority_queue<unsigned, std::vector<unsigned>, PQwmapcompare> *vpq = new
    std::priority_queue<unsigned, std::vector<unsigned>, PQwmapcompare>
    ( PQwmapcompare ( &data_, pt_ ) );
    for ( unsigned k = 0; k < size_; ++k ) vpq->push ( k );
    unsigned k = 0;
    ptr_ = new unsigned[size_];
    while ( !vpq->empty() ) {
      ptr_[k++] = vpq->top();
      vpq->pop();
    }
    delete vpq;
    LINFO ( "Done" );
  };

  /**
   * Binary search of a needle in our wordmap string, between low and high.
   * This is a recursive implementation.
   * \param sn: String to make the search (typically the wordmap)
   * \param needle: Word we are looking for
   * \param low: Low boundary index
   * \param high: high boundary index
   */
  int bs ( std::string& sn, std::string needle, unsigned low, unsigned high ) {
    if ( high < low )  return -1; // not found
    int mid = low + ( high - low ) / 2;
    if ( strncmp ( sn.c_str() + pt_[ptr_[mid]], needle.c_str(),
                   needle.size() ) > 0 ) return bs ( sn, needle, low, mid - 1 );
    else if ( strncmp ( sn.c_str() + pt_[ptr_[mid]], needle.c_str(),
                        needle.size() ) < 0 ) return bs ( sn, needle, mid + 1, high );
    return mid; // found
  }

  /**
   * Word-maps a sequence of integers (represented as a string).
   * \param is: Input string containing a sequence of integers. Note that it is not passed as const reference -- a copy would be needed in order to do trim.
   * \param os: Output string containing a sequence of words.
   */

  void mapi2str ( std::string is, std::string *os ) {
    boost::algorithm::trim ( is );
    *os = "";
    if ( is == "" ) return;
    std::vector<std::string> words;
    boost::algorithm::split ( words, is, boost::algorithm::is_any_of ( " " ) );
    for ( unsigned k = 0; k < words.size(); ++k ) {
      unsigned index = toNumber<unsigned> ( words[k] );
      if ( index >= size_ ) {
        if ( index == OOV || index == DR ) continue;
        LINFO ( "idx OOV detected:" << index );
        USER_CHECK ( oovwmap_.find ( index ) != oovwmap_.end(),
                     "OOV index does not exist in the word map!" );
        *os += oovwmap_[ index ] + " ";
        continue;
      }
      if ( index < size_ - 1 )
        *os += data_.substr ( pt_[index], pt_[index + 1] - pt_[index] - 1 ) + " ";
      else
        *os += data_.substr ( pt_[index], data_.size() - 1  ) + " ";
    }
  };

  /**
   * Integer-maps a sentence of words
   * \param is: Input string containing a sequence of words. Note that it is not passed as const reference -- a copy would be needed in order to do trim.
   * \param os: Output string containing a sequence of integers.
   */

  void mapstr2i ( std::string is, std::string *os ) {
    USER_CHECK ( reverse_, "Reverse search not implemented for this object." );
    boost::algorithm::trim ( is );
    *os = "";
    if ( is == "" ) return;
    std::vector<std::string> words;
    boost::algorithm::split ( words, is, boost::algorithm::is_any_of ( " " ) );
    for ( unsigned k = 0; k < words.size(); ++k ) {
      int result = this->bs ( data_, words[k] + "\n" , 0, size_ - 1 );
      if ( result < 0 ) { // OOVs handler.
        if ( oovrwmap_.find ( words[k] ) == oovrwmap_.end() ) {
          oovwmap_[ oovid_ ] = words[k];
          oovrwmap_[words[k]] = oovid_++ ;
        }
        *os += toString<std::size_t> ( oovrwmap_[words[k]] ) + " ";
        continue;
      }
      *os += toString<unsigned> ( ptr_[result] ) + " ";
    }
  };
  DISALLOW_COPY_AND_ASSIGN ( WordMapper );

};

}
}  // end namespaces

#endif
