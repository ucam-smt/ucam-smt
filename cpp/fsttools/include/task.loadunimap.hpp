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

#ifndef TASK_LOADUNIMAP_HPP
#define TASK_LOADUNIMAP_HPP

/**
 * \file
 * \brief Implementation of a unigram transduction model loader task
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

#include "task.disambig.flowerfst.hpp"

namespace ucam {
namespace fsttools {

/**
 * \brief Loads a unigram transduction model (aka unimap file) from a file with the format accepted by srilm disambig tool
 */
template<class Data, class Arc = fst::StdArc >
class LoadUnimapTask : public ucam::util::TaskInterface<Data> {

  typedef typename Arc::Weight Weight;
  typedef typename Arc::Label Label;

 private:

  ///Name of unimap file
  std::string unimapfile_;
  ///Pointer to the unimap fst
  fst::VectorFst<Arc> *unimap_;
  ///Scale applied to the unimap model
  float uscale_;
  ///Target vocabulary
  std::unordered_set<std::string> vcblm_;

  ///Registry object with user parameters
  const ucam::util::RegistryPO& rg_;

  //Key for a place to store a pointer to the unimap fst in the data object
  const std::string unimapkey_;

  ///Weight creator, compatible with various semirings, i.e. tropical and lexicographic<tropical,tropical>
  fst::MakeWeight2<Arc> mw_;

  ///has already been loaded or not
  bool loaded_;

 public:

  /**
   * \brief Constructor.
   * \param rg: Object containing user params
   * \param unimapkey: key to deliver the unimap model in the data object
   */
  LoadUnimapTask ( const ucam::util::RegistryPO& rg,
                   const std::string& unimapkey = HifstConstants::kRecaserUnimapLoad,
                   const std::string& lmkey = HifstConstants::kRecaserLmLoad
                 ) :
    rg_ ( rg ),
    unimapkey_ ( unimapkey ),
    unimapfile_ ( rg.get<std::string> ( unimapkey ) ),
    uscale_ ( rg.get<float> ( "recaser.unimap.scale" ) ),
    loaded_ ( false ),
    unimap_ ( NULL ) {
    if ( rg.get<std::string> ( lmkey ) == ""
         && rg.get<std::string> ( unimapkey ) == "" ) return;
    if ( ! USER_CHECK ( rg.get<std::string> ( lmkey ) != ""
                        && rg.get<std::string> ( unimapkey ) != ""
                        , "recaser.lm.load and recaser.unimap.load must either be both defined or both left to empty string " ) )
      return;
  };

  /**
   * \brief Loads unimap fst, and delivers pointer in data object.
   * \param d; Templated data object
   */
  bool run ( Data& d ) {
    load();
    LINFO ( "Unimap model available at key=" << unimapkey_ );
    d.fsts[unimapkey_] = unimap_;
    d.recasingvcblm = &vcblm_;
    return false;
  };

  ///Destructor
  virtual ~LoadUnimapTask() {
    delete unimap_;
  }

 private:

  ///Implements the actual loading of the unimap model
  inline void load() {
    if ( loaded_ ) return;
    if ( unimapfile_ == "" ) return;
    LINFO ( "Read Unigram Model" );
    ucam::util::iszfstream umf ( unimapfile_ );
    unimap_ = new fst::VectorFst<Arc>;
    loadflowerfst<Arc> ( umf, *unimap_ );
    umf.close();
    LINFO ( "Applying uscale=" << uscale_ );
    SetGsf<Arc> ( unimap_, uscale_ );
    fst::extractTargetVocabulary<Arc> ( *unimap_, &vcblm_ );
    LDBG_EXECUTE ( unimap_->Write ( "unimap.fst" ) );
    loaded_ = true;
  }

};

}
} // end namespaces

#endif // TASK_LOADUNIMAP_HPP

