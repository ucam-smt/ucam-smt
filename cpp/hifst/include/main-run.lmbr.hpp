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

#ifndef MAIN_RUN_LMBR_HPP
#define MAIN_RUN_LMBR_HPP

/**
 * \file
 * \brief Implements single-threaded version of alilats2splats tool
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace lmbr {

/**
 * \brief Full single-threaded Alignment lattices to Sparse lattices
 */

template <class Data = LmbrTaskData >
class SingleThreadedLmbrTask: public ucam::util::TaskInterface<Data> {

 private:
  typedef ucam::util::TaskInterface<Data> ITask;
  typedef ucam::fsttools::WriteFstTask<Data> WriteFst;
  typedef ucam::lmbr::LmbrTask <Data > Lmbr;
  const ucam::util::RegistryPO& rg_;
 public:
  /**
   *\brief Constructor
   *\param rg: pointer to RegistryPO object with all parsed parameters.
   */
  SingleThreadedLmbrTask ( const ucam::util::RegistryPO& rg ) :
    rg_ ( rg ) {
  };

  /**
   * \brief Reads evidence space and hypotheses space (FSTs) and applies lmbr.
   */
  bool run ( Data& d ) {
    using ucam::fsttools::ReadFstInit;
    using ucam::util::oszfstream;
    using ucam::util::toString;
    std::string const& smr = rg_.exists (HifstConstants::kLmbrLexstdarc) ?
                             HifstConstants::kHifstSemiringLexStdArc : HifstConstants::kHifstSemiringStdArc;
    boost::scoped_ptr < ITask > mytask ( ReadFstInit<Data> ( rg_
                                         , HifstConstants::kLmbrLoadEvidencespace
                                         , smr ) ); // Read evidence space
    mytask->appendTask
    ( ReadFstInit<Data> ( rg_
                          , HifstConstants::kLmbrLoadHypothesesspace
                          , smr ) )
    ( new Lmbr ( rg_  ) )
    ( WriteFst::init ( rg_  , HifstConstants::kLmbrWritedecoder ) )
    ;
    unordered_map<std::string, boost::shared_ptr<oszfstream > > onebestfiles;
    ucam::util::PatternAddress<float> onebestfilename (rg_.get<std::string>
        (HifstConstants::kLmbrWriteonebest), "%%alpha%%");
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                      HifstConstants::kRangeOne ) );
          !ir->done ();
          ir->next () ) {
      lmbrtunedata lmbronebest;
      d.sidx = ir->get ();
      d.lmbronebest = &lmbronebest;
      mytask->chainrun ( d );        // Run!
      if (rg_.exists (HifstConstants::kLmbrWriteonebest) ) {
        for (unsigned j = 0; j < d.lmbronebest->alpha.size(); ++j) {
          std::string filename = onebestfilename (d.lmbronebest->alpha[j]);
          ucam::util::find_and_replace (filename, "%%wps%%",
                                        toString<float> (d.lmbronebest->wps[j]  ) );
          ucam::util::find_and_replace (filename, "?",
                                        toString<unsigned> (d.lmbronebest->idx  ) );
          if (onebestfiles.find (filename) == onebestfiles.end() )
            onebestfiles[filename] = boost::shared_ptr<oszfstream> (new oszfstream (
                                       filename) );
          *onebestfiles[filename] << d.lmbronebest->alpha[j]
                                  << " " << d.lmbronebest->wps[j]
                                  << " " << d.lmbronebest->idx
                                  << ":" << d.lmbronebest->hyp[j] << endl;
        }
      }
    }
    return false;
  };

  inline bool operator() () {
    Data d;
    return run ( d );
  };

 private:

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadedLmbrTask );

};

/**
 * \brief Not implemented yet!
 */
template <class Data = LmbrTaskData >
class MultiThreadedLmbrTask: public ucam::util::TaskInterface<Data> {
 private:
  typedef ucam::util::TaskInterface<Data> ITask;
  typedef ucam::fsttools::WriteFstTask<Data> WriteFst;
  typedef ucam::lmbr::LmbrTask <Data > Lmbr;

  ///Registry object
  const ucam::util::RegistryPO& rg_;

  ///Number of threads requested by user
  unsigned threadcount_;
 public:
  MultiThreadedLmbrTask ( const ucam::util::RegistryPO& rg ) :
    rg_ (rg),
    threadcount_ ( rg.get<unsigned> ( HifstConstants::kNThreads.c_str() ) ) {
  };

  ///original_data is being ignored in this case.
  bool run ( Data& original_data ) {
    using ucam::fsttools::ReadFstInit;
    using ucam::util::TaskInterface;
    using ucam::util::TaskFunctor;
    using ucam::util::oszfstream;
    using ucam::util::toString;
    std::vector < boost::shared_ptr< lmbrtunedata > > lmbronebest;
    std::string const& smr = rg_.exists (HifstConstants::kLmbrLexstdarc)
                             ? HifstConstants::kHifstSemiringLexStdArc
                             : HifstConstants::kHifstSemiringStdArc;
    {
      ucam::util::TrivialThreadPool tp ( threadcount_ );
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ,
                                        HifstConstants::kRangeOne ) )
            ; !ir->done()
            ; ir->next() ) {
        TaskInterface<Data> *mytask (ReadFstInit <Data > ( rg_ ,
                                     HifstConstants::kLmbrLoadEvidencespace ,
                                     smr)
                                    ); //Read evidence space.
        mytask->appendTask
        ( ReadFstInit<Data> ( rg_
                              , HifstConstants::kLmbrLoadHypothesesspace
                              , smr ) )
        ( new Lmbr ( rg_  ) )
        ( WriteFst::init ( rg_  ,
                           HifstConstants::kLmbrWritedecoder ) )
        ;
        Data *d = new Data;
        d->sidx = ir->get();
        LINFO ("Processing sentence " << d->sidx);
        lmbronebest.push_back ( boost::shared_ptr< lmbrtunedata >
                                ( new lmbrtunedata  ) );
        d->lmbronebest = lmbronebest[lmbronebest.size() - 1].get();
        tp ( TaskFunctor<Data> ( mytask, d ) ); //Handles pointer ownership
      }
    }
    if (rg_.exists (HifstConstants::kLmbrWriteonebest.c_str() ) ) {
      //Write all 1-bests
      unordered_map<std::string, boost::shared_ptr<oszfstream > > onebestfiles;
      ucam::util::PatternAddress<float> onebestfilename (rg_.get<std::string>
          (HifstConstants::kLmbrWriteonebest), "%%alpha%%");
      FORCELINFO ("Writing to file(s) one best hypotheses");
      for (unsigned k = 0; k < lmbronebest.size(); ++k) {
        for (unsigned j = 0; j < lmbronebest[k]->alpha.size(); ++j) {
          std::string filename = onebestfilename (lmbronebest[k]->alpha[j]);
          ucam::util::find_and_replace (filename, "%%wps%%",
                                        toString<float> (lmbronebest[k]->wps[j]  ) );
          ucam::util::find_and_replace (filename, "?",
                                        toString<unsigned> (lmbronebest[k]->idx  ) );
          if (onebestfiles.find (filename) == onebestfiles.end() )
            onebestfiles[filename] = boost::shared_ptr<oszfstream> (new oszfstream (
                                       filename) );
          *onebestfiles[filename] << lmbronebest[k]->alpha[j]
                                  << " " << lmbronebest[k]->wps[j]
                                  << " " << lmbronebest[k]->idx
                                  << ":" << lmbronebest[k]->hyp[j] << endl;
        }
      }
    }
    return false;
  };

  ///Runs translation with an internal data object.
  inline bool operator() () {
    Data d;
    return run ( d );
  }

 private:
  DISALLOW_COPY_AND_ASSIGN ( MultiThreadedLmbrTask );
};

}
}  //  end namespaces

#endif
