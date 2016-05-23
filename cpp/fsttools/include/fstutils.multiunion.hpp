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

#ifndef FSTUTILS_MULTIUNION_HPP
#define FSTUTILS_MULTIUNION_HPP

/** \file
 * \brief Implementations of multiple fst unions
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */

namespace fst {

/**
 * \brief This class creates the Union of an arbitrarily large number of fsts.
 * This implementation was suggested by Cyril (13-08-2009), as a more efficient alternative to using consecutive Union() over two VectorFsts.
 * The reason being that this generates internally a list of ReplaceFsts.
 */
template<class Arc>
class MultiUnionRational {

 private:

  ///List of fsts to be unioned
  std::vector< boost::shared_ptr< Fst<Arc> const > > fsts_;

 public:
  ///Empty Constructor
  MultiUnionRational() {};

  /**
   * \brief Adds an fst to the list
   * \param fst: Pointer to another fst to be unioned with previously stored ones.
   */
  inline void Add ( boost::shared_ptr< Fst<Arc> const > fst ) {
    fsts_.push_back ( fst );
  };

  ///Also adds an fst, but takes a pointer as input. Note that we are using internally a list of shared_ptr that take ownership of these pointers
  inline void Add ( Fst<Arc> const *fst ) {
    fsts_.push_back ( boost::shared_ptr< Fst<Arc> const > ( fst ) );
  };

  /*
   * \brief Proceeds to create the union of list of fsts contained so far in fsts_.
   * Note that it is possible to create intermediate union-fsts.
   */
  VectorFst<Arc> *operator () () {
    if ( !fsts_.size() ) return NULL;
    VectorFst<Arc> * aux2 = NULL;
    if ( fsts_.size() > 1 ) {
      boost::scoped_ptr< UnionFst<Arc> > aux ( new UnionFst<Arc> ( *fsts_[0],
          *fsts_[1] ) );
      for ( uint k = 2; k < fsts_.size(); ++k ) Union ( & ( *aux ), *fsts_[k] );
      aux2 = new VectorFst<Arc> ( *aux );
    } else aux2 = new VectorFst<Arc> ( *fsts_[0] );
    return aux2;
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( MultiUnionRational );

};

/**
 * \brief This class creates the Union of an arbitrarily large number of fsts.
 * This implementation uses one RTN to generate the union.
 */
template<class Arc >
class MultiUnionReplace {
 private:
  uint counter_;
  const uint unionindex_;
  VectorFst<Arc> head_;

  // List of pairs label/FSA.
  std::vector< std::pair< typename Arc::Label, const Fst<Arc> * > > pairlabelfsts_;
  std::vector<boost::shared_ptr< Fst<Arc> const > > fsts_;

 public:
  ///Constructor, initializes the head fst of the RTN.
  MultiUnionReplace() :
    counter_ ( 1 ),
    unionindex_ ( 1000000000 ) {
    head_.AddState();
    head_.AddState();
    head_.SetStart ( 0 );
    head_.SetFinal ( 1, Arc::Weight::One() );
    pairlabelfsts_.push_back ( std::pair< typename Arc::Label, const Fst<Arc> * >
                               ( unionindex_, &head_ ) );
  };

  /**
   * \brief Adds an fst to the list
   * \param fst: Pointer to another fst to be unioned with previously stored ones.
   */
  inline void Add ( Fst<Arc> const *fst ) {
    ///Keep in scope the pointers
    fsts_.push_back ( boost::shared_ptr< Fst<Arc> const > ( fst ) );
    ///Add pair labels for Replace operation
    head_.AddArc ( 0, Arc ( unionindex_ + counter_, unionindex_ + counter_,
                            Arc::Weight::One(), 1 ) );
    pairlabelfsts_.push_back ( std::pair< typename Arc::Label, const Fst<Arc> * >
                               ( unionindex_ + counter_, fst ) );
    ++counter_;
  };

  /*
   * \brief Proceeds to create the union of list of fsts contained so far in fsts_.
   * Note that it is possible to create intermediate union-fsts.
   */
  VectorFst<Arc> * operator () () {
    return new VectorFst<Arc> ( ReplaceFst<Arc> ( pairlabelfsts_,
                                ReplaceFstOptions<Arc> ( unionindex_, true ) ) );
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( MultiUnionReplace );
};

} //end namespace

#endif
