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

#ifndef APPLYLMTASK_HPP
#define APPLYLMTASK_HPP

/**
 * \file
 * \brief Implementation of a language model task
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/**
 * \brief Language model loader task, loads a language model wrapping it in a class to provide.
 *
 */

template <class Data , class KenLMModelT,  class Arc = fst::StdArc >
class ApplyLanguageModelTask: public ucam::util::TaskInterface<Data> {

  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:

  bool deletelmscores_;
  bool natlog_;

  const std::string lmkey_;
  const std::string latticeloadkey_;
  const std::string latticestorekey_;

  fst::VectorFst<Arc> mylmfst_;

 public:
  ///Constructor with ucam::util::RegistryPO object
  ApplyLanguageModelTask ( const ucam::util::RegistryPO& rg ,
                           const std::string& lmkey = HifstConstants::kLmLoad ,
                           const std::string& latticeloadkey = "lm.lattice.load",
                           const std::string& latticestorekey = "lm.lattice.store",
                           bool deletelmscores = false
                         ) :
    lmkey_ ( lmkey ),
    latticeloadkey_ ( latticeloadkey ),
    latticestorekey_ ( latticestorekey ),
    natlog_ ( !rg.exists ( HifstConstants::kLmLogTen ) ),
    deletelmscores_ (deletelmscores) {
  };

  /**
   * \brief Method inherited from ucam::util::TaskInterface. Loads the language model and stores in lm data structure.
   * \param &d: data structure in which the null filter is to be stored.
   * \returns false (does not break the chain of tasks)
   */
  bool run ( Data& d ) {
    mylmfst_.DeleteStates();
    if ( !USER_CHECK ( d.klm.size() ,
                       "No language models available" ) ) return true;
    if ( !USER_CHECK ( d.klm.find ( lmkey_ ) != d.klm.end() ,
                       "No language models available (key not initialized) " ) ) return true;
    if ( !USER_CHECK ( d.fsts.find ( latticeloadkey_ ) != d.fsts.end() ,
                       " Input fst not available!" ) ) return true;
    mylmfst_ = * (static_cast<fst::VectorFst<Arc> * > ( d.fsts[latticeloadkey_] ) );
    if (deletelmscores_) {
      LINFO ( "Delete old LM scores first" );
      //Deletes LM scores if using lexstdarc. Note -- will copy through on stdarc and ignore on tuplearc!
      fst::MakeWeight2<Arc> mwcopy;
      fst::Map<Arc> ( &mylmfst_,
                      fst::GenericWeightAutoMapper<Arc, fst::MakeWeight2<Arc> > ( mwcopy ) );
    }
    LINFO ( "Input lattice loaded with key=" << latticeloadkey_ << ", NS=" <<
            mylmfst_.NumStates() );
    fst::MakeWeight<Arc> mw;
    for ( uint k = 0; k < d.klm[lmkey_].size(); ++k ) {
      if ( !USER_CHECK ( d.klm[lmkey_][k]->model != NULL,
                         "Language model " << k << " not available!" ) ) return true;
      KenLMModelT& model = *d.klm[lmkey_][k]->model;
#ifndef USE_GOOGLE_SPARSE_HASH
      unordered_set<Label> epsilons;
#else
      google::dense_hash_set<Label> epsilons;
      epsilons.set_empty_key ( numeric_limits<Label>::max() );
#endif
      ///We want the language model to ignore these guys:
      epsilons.insert ( DR );
      epsilons.insert ( OOV );
      epsilons.insert ( EPSILON );
      epsilons.insert ( SEP );
      LINFO ( "Applying language model " << k
              << " with lmkey=" << lmkey_
              << ", using lmscale=" << d.klm[lmkey_][k]->lmscale );
      LDEBUG ( "lattice NS=" << mylmfst_.NumStates() );
      fst::ApplyLanguageModelOnTheFly<Arc, fst::MakeWeight<Arc>, KenLMModelT> *f
        = new fst::ApplyLanguageModelOnTheFly<Arc, fst::MakeWeight<Arc>, KenLMModelT >
      ( mylmfst_
        , model
        , epsilons
        , natlog_
        , d.klm[lmkey_][k]->lmscale
        , d.klm[lmkey_][k]->lmwp
        , d.klm[lmkey_][k]->idb);
      f->setMakeWeight ( mw );
      d.stats->setTimeStart ( "on-the-fly-composition " +  ucam::util::toString (
                                k ) );
      mylmfst_ = * ( ( *f ) () );
      delete f;
      d.stats->setTimeEnd ( "on-the-fly-composition " + ucam::util::toString ( k ) );
      LDEBUG ( mylmfst_.NumStates() );
      mw.update();
    }
    d.fsts[latticestorekey_] = &mylmfst_;
    LINFO ( "Done!" );
    return false;
  };

  ~ApplyLanguageModelTask ( ) {
    LINFO ("Shutdown!");
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( ApplyLanguageModelTask );

};

}
}  // end namespaces

#endif
