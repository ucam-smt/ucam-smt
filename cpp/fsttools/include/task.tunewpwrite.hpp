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

#ifndef TASK_TUNEWPWRITEFST_HPP
#define TASK_TUNEWPWRITEFST_HPP

/**
 * \file
 * \brief Tune word penalty and write output
 * \date 25-05-2015
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/**
 * \brief Convenience class that inherits Taskinterface behaviour and writes
 * an fst to [file] using a key defined in the constructor. The key is used
 * to access the registry object (i.e. actual program option telling where
 * to write the fst) and a pointer in the data object, telling where to read
 * the fst from.
 */
template <class Data, class Arc = fst::StdArc >
class TuneWpWriteFstTask: public ucam::util::TaskInterface<Data> {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;
  typedef std::vector<std::pair<Label, Label> > VectorPair;
 private:
  ///key to access fst in the data object
  std::string fstkey_;
  std::string readfstkey_;
  ///Fst filename
  ucam::util::IntegerPatternAddress fstfile_;
  ucam::util::NumberRange<float> wp_;
  std::unordered_set<Label> epsilons_;
  bool doTune_;
  fst::MakeWeight<Arc> mw_;
 public:
  TuneWpWriteFstTask ( const ucam::util::RegistryPO& rg
                 , const std::string& fstkey
                 , const std::string& readfstkey = ""
               )
      : fstkey_ ( fstkey )
      , readfstkey_ (readfstkey != "" ? readfstkey : fstkey)
      , fstfile_ ( rg.get<std::string> ( fstkey ) )
      , wp_( rg.get<std::string>(HC::kTuneWordPenaltyRange ) )
      , doTune_(rg.getBool(HC::kTune) )
  {
    LDEBUG("Init TuneWpWriteFstTask: " << fstfile_() );
    epsilons_.insert(OOV);
    epsilons_.insert(EPSILON);
    epsilons_.insert(DR);
  };

  inline static TuneWpWriteFstTask *init ( const ucam::util::RegistryPO& rg
                                           , const std::string& fstkey
                                           , const std::string& readfstkey = ""
                                      ) {
    if ( rg.exists ( fstkey )
         && rg.getBool(HC::kTune) )
      return new TuneWpWriteFstTask ( rg, fstkey, readfstkey );
    return NULL;
  };

  /**
   * \brief Method inherited from TaskInterface. Stores fst to [file].
   * The fst is accessed via data object using access key fstkey_.
   * PDAs not supported.
   * \param &d: data object
   * \returns false (does not break in any case the chain of tasks)
   */
  inline bool run ( Data& d ) {
    if ( fstfile_ ( d.sidx ) == "" ) return false;
    if ( d.fsts.find ( readfstkey_ ) == d.fsts.end() ) {
      LERROR ( "fst with key="  << readfstkey_ << " does not exist!" );
      exit ( EXIT_FAILURE );
    }
    using namespace ucam::util;
    using namespace fst;

    LDEBUG("Word penalty application --");
    for ( wp_.start(); !wp_.done(); wp_.next() ) {
      VectorFst<Arc> mfst (*( static_cast< Fst<Arc> *> (d.fsts[readfstkey_]) ) );
      LDEBUG("w=" << wp_.get());
      Map<Arc, WordPenaltyMapper<Arc> >
          (&mfst, WordPenaltyMapper<Arc> (mw_ (wp_.get() ), epsilons_) );
      VectorFst<Arc> aux;
      ShortestPath<Arc> (mfst, &aux);
      mfst = aux;
      std::string auxs= fstfile_(d.sidx);
      find_and_replace (auxs, HC::kUserWpRange, toString<float> (wp_() ) );
      FstWrite<Arc> ( mfst, auxs);
      FORCELINFO("Wrote " << auxs );
    }
    return false;
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( TuneWpWriteFstTask );
};

}}  // end namespaces

#endif
