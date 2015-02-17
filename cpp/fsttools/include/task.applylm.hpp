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
 * \brief Implementation of a language model application task
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <task.applylm.kenlmtype.hpp>


namespace ucam {
namespace fsttools {

/**
 * \brief Language model loader task, loads a language model wrapping it in a class to provide.
 *
 */
template <class Data ,  class Arc >
class ApplyLanguageModelTask: public ucam::util::TaskInterface<Data> {

  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:
  ucam::util::RegistryPO const& rg_;
  bool deletelmscores_;
  bool natlog_;

  const std::string lmkey_;
  const std::string latticeloadkey_;
  const std::string latticestorekey_;

  fst::VectorFst<Arc> mylmfst_;


  typedef fst::ApplyLanguageModelOnTheFlyInterface<Arc> ApplyLanguageModelOnTheFlyInterfaceType;
  typedef boost::shared_ptr<ApplyLanguageModelOnTheFlyInterfaceType> ApplyLanguageModelOnTheFlyInterfacePtrType;
  std::vector<ApplyLanguageModelOnTheFlyInterfacePtrType> almotf_;

 public:
  ///Constructor with ucam::util::RegistryPO object
  ApplyLanguageModelTask ( const ucam::util::RegistryPO& rg ,
                           const std::string& lmkey = HifstConstants::kLmLoad ,
                           const std::string& latticeloadkey = "lm.lattice.load",
                           const std::string& latticestorekey = "lm.lattice.store",
                           bool deletelmscores = false
                         )
    : rg_(rg)
    , lmkey_ ( lmkey )
    , latticeloadkey_ ( latticeloadkey )
    , latticestorekey_ ( latticestorekey )
    , natlog_ ( !rg.exists ( HifstConstants::kLmLogTen ) )
    , deletelmscores_ (deletelmscores) {
  };

  /**
   * \brief Initializes appropriate templated handlers for kenlm language models
   */
 void initializeLanguageModelHandlers(Data &d) {
   if (almotf_.size()) return; // already done
   almotf_.resize(d.klm[lmkey_].size());
   fst::MakeWeight<Arc> mw;
   unordered_set<Label> epsilons;
   /// We want the language model to ignore these guys:
   epsilons.insert ( DR );
   epsilons.insert ( OOV );
   epsilons.insert ( EPSILON );
   epsilons.insert ( SEP );
   for ( unsigned k = 0; k < d.klm[lmkey_].size(); ++k ) {
     USER_CHECK ( d.klm[lmkey_][k]->model != NULL,
		  "Language model " << k << " not available!" );
     almotf_[k].reset(assignKenLmHandler<Arc>(rg_,lmkey_, epsilons
					      , *(d.klm[lmkey_][k])
					      , mw, natlog_,k));
     
     mw.update();
   }
 }

  /**
   * \brief Method inherited from ucam::util::TaskInterface. Loads the language model and stores in lm data structure.
   * \param &d data structure in which the null filter is to be stored.
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
    initializeLanguageModelHandlers(d);
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
    boost::shared_ptr<fst::VectorFst<Arc> > p;
    for ( unsigned k = 0; k < almotf_.size(); ++k ) {
      d.stats->setTimeStart ( "on-the-fly-composition " +  ucam::util::toString ( k ) );
      p.reset(almotf_[k]->run(mylmfst_));
      mylmfst_ = *p;
      p.reset();
      d.stats->setTimeEnd ("on-the-fly-composition " + ucam::util::toString ( k ) );
      LDEBUG ( mylmfst_.NumStates() );
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
