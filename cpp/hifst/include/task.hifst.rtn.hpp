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

#ifndef TASK_HIFST_RTN_HPP
#define TASK_HIFST_RTN_HPP
/**
 * \file
 * \brief Implements RTN class.
 * Stores pointers to cell FSAs of the RTN using a hiero-index representing cell coordinates (cc,x,y)
 * \date 1-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Convenience class that stores pointers to cell FSAs.
 * These pointers are organized through a hash using a label built from (cc,x,y) as key
 */

template<class Arc>
class RTN {

  typedef typename Arc::Label Label;
  typedef std::tr1::unordered_map< Label , boost::shared_ptr< fst::Fst<Arc> > >
  hifst_rtn_t;
 private:
  ///rtn hash
  hifst_rtn_t rtn_ , rtn2_;

 public:
  ///constructor
  RTN() {};

  ///If exists, returns pointer FSA for a given key. Otherwise returns NULL
  inline fst::Fst<Arc> *operator() ( const unsigned cc, const unsigned x, const unsigned y ) {
    Label hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    typename hifst_rtn_t::iterator itx = rtn_.find ( hieroindex );
    if ( itx != rtn_.end() )
      return itx->second.get();
    return NULL;
  };

  ///Adds a new cell fsa to the rtn for coordinates cc,x,y, takes ownership of the pointer. fst is the pointer arc and fst2 contains the pointee fst.
  inline void Add ( const unsigned cc,
                    const unsigned x,
                    const unsigned y,
                    fst::Fst<Arc> *fst ,
                    fst::Fst<Arc> *fst2 = NULL) {
    Label hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    rtn_[hieroindex] = boost::shared_ptr<fst::Fst<Arc> > ( fst );
    if (fst2 != NULL)
      rtn2_[hieroindex] = boost::shared_ptr<fst::Fst<Arc> > ( fst2 );
  };
  ///Adds a new cell fsa to the rtn for coordinates cc,x,y
  inline void Add ( const unsigned cc,
                    const unsigned x,
                    const unsigned y,
                    boost::shared_ptr< fst::Fst<Arc> > fst ,
                    boost::shared_ptr< fst::Fst<Arc> > fst2 ) {
    Label hieroindex = APBASETAG + cc * APCCTAG + x * APXTAG + y * APYTAG;
    rtn_[hieroindex] = fst;
    rtn2_[hieroindex] = fst2;
  };

  ///Clears hash
  inline void clear() {
    rtn_.clear();
  };

  ///Returns size (number of FSAs) in the RTN/cyk grid
  inline std::size_t size() {
    return rtn_.size();
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( RTN );
};

}
} // end namespaces

#endif //TASK_HIFST_RTN_HPP
