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

/** \file include/range.hpp
 * \brief Handles different type of integer ranges.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef RANGEIMPLEMENTATIONS_HPP
#define RANGEIMPLEMENTATIONS_HPP

namespace HifstConstants {
const std::string kRange = "range";
const std::string kRangeOne = "one";
const std::string kRangeInfinite = "infinite";
}

namespace ucam {
namespace util {

/**
 * \brief Generates ranges  from a compact string parameter such as 1,3:5,10
 * \param range A key string such as 1,3:5,10, describing a range of values.
 * \param x A vector containing explicitly the integers we care about
 */

template<typename NumberType>
inline void getRange ( const std::string& range, std::vector<NumberType>& x ) {
  std::vector<std::string> aux;
  boost::algorithm::split ( aux, range, boost::algorithm::is_any_of ( "," ) );
  for ( uint i = 0; i < aux.size(); ++i ) {
    std::string& s = aux[i];
    if ( !exists ( s, ":" ) ) {
      x.push_back ( toNumber<NumberType> ( s ) );
      continue;
    }
    std::vector<std::string> range_aux;
    boost::algorithm::split ( range_aux, s, boost::algorithm::is_any_of ( ":" ) );
    USER_CHECK ( range_aux.size()
                 && range_aux.size() <= 3, "string range incorrectly defined!" );
    NumberType jump = 1;
    if ( range_aux.size() == 3 ) jump = toNumber<NumberType> ( range_aux[1] );
    NumberType start = toNumber<NumberType> ( range_aux[0] );
    NumberType  end = toNumber<NumberType> ( range_aux[range_aux.size() - 1] );
    for ( NumberType k = start; k <= end; k += jump ) {
      x.push_back ( k );
    }
  }
};

/**
 *\brief Interface for an arbitrary range of numbers
 */

template<class NumberType>
class NumberRangeInterface {
 public:
  virtual void next ( void ) = 0;
  virtual bool done ( void ) = 0;
  virtual void start ( void ) = 0;
  virtual NumberType get ( void ) = 0;
  virtual ~NumberRangeInterface() {};
};

template<class NumberType = uint>
class NumberRange : public NumberRangeInterface<NumberType> {
 private:
  /// arbitrary sequence of numbers
  std::vector<NumberType> range_;
  /// Current position.
  uint k_;
 public:
  /**
   *\brief Constructor
   * \param r vector of unsigned integers
   */

  NumberRange ( const std::vector<NumberType>& r ) : range_ ( r ), k_ ( 0 ) {};

  /**
   *\brief Constructor
   * \param rg pointer to RegistryPO objects allowing access to command-line/config file options
   */

  NumberRange ( const RegistryPO& rg ,
                const std::string& rangekey = HifstConstants::kRange) : k_ ( 0 ) {
    getRange ( rg.get<std::string> ( rangekey ), range_ );
  };

  NumberRange ( const std::string& range) : k_ ( 0 ) {
    getRange ( range , range_ );
  };

  ///Empty implementation
  inline void start ( void ) {
    k_ = 0;
  };
  /// Increment index.
  inline void next ( void ) {
    if ( !done() ) ++k_;
  };
  /// Checks if reached the last element
  inline bool done ( void ) {
    return k_ >= range_.size();
  };
  /// Returns range value at position k_.
  inline NumberType get ( void ) {
    return range_.at ( k_ );
  };
  inline NumberType operator() ( void ) {
    return range_.at ( k_ );
  };

 private:

  DISALLOW_COPY_AND_ASSIGN ( NumberRange );

};

/**
 *\brief Implements a Range iterator that only runs once.
 * This is useful e.g. for fsttools that process a batch of files if range is specificied, and only one if not.
 */

template <typename NumberType>
class OneRange : public NumberRangeInterface<NumberType> {
 private:
  ///  sentence index value.
  NumberType sid_;
  bool state_;
 public:
  OneRange ( const NumberType u = 0 ) : sid_ ( u ) , state_ ( false ) {};
  ///Empty implementation
  inline void start ( void ) {
    state_ = false;
  };
  ///Empty implementation
  inline void next ( void ) {
    state_ = true;
  };

  ///return true.
  inline bool done ( void ) {
    return state_;
  };
  ///Always returns sid_
  inline NumberType get ( void ) {
    return sid_;
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( OneRange );

};

/**
 *\brief Implements a Range iterator that will never finish.
 */

template<typename NumberType = uint >
class InfiniteRange : public NumberRangeInterface<NumberType> {
 private:
  ///  sentence index value.
  NumberType sid_;
 public:
  InfiniteRange ( const NumberType u = 1 ) : sid_ ( u ) {};
  ///Empty implementation
  inline void start ( void ) {};
  ///Empty implementation
  inline void next ( void ) {
    ++sid_;
  };

  ///Always return false
  inline bool done ( void ) {
    return false;
  };
  ///Always returns sid_
  inline NumberType get ( void ) {
    return sid_;
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( InfiniteRange );

};

template<typename NumberType>
NumberRangeInterface<NumberType>* RangeInitFactory ( const RegistryPO& rg ,
    const std::string& option = HifstConstants::kRangeInfinite ) {
  if ( !rg.exists ( HifstConstants::kRange ) )
    if ( option == HifstConstants::kRangeInfinite )
      return new InfiniteRange<NumberType>();
  //this range will never break
    else if ( option == HifstConstants::kRangeOne )
      return new OneRange<NumberType>();
  //Just run once if range not specified
  return new NumberRange<NumberType> ( rg );
};

#define IntRangeFactory RangeInitFactory<unsigned>
typedef boost::scoped_ptr < NumberRangeInterface<unsigned> > IntRangePtr;

}
}  // end namespaces

#endif

