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

/** \file
 *  \brief Describes class ReferenceFilterTask (builds unweighted substring fst for lattice alignment )
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef TASK_REFERENCE_FST_HPP
#define TASK_REFERENCE_FST_HPP

namespace ucam {
namespace hifst {

/**
 * \brief Generates a substring version of a reference translation lattice and associated vocabulary.
 * This substring fst is typically used to guide translation towards a particular search space.
 * The associated vocabulary can be used e.g. to restrict parsing algorithms.
 */
template <class Data , class Arc = fst::LexStdArc >
class ReferenceFilterTask: public ucam::util::TaskInterface<Data> {

  typedef typename Arc::Weight Weight;
  typedef typename Arc::Label Label;

  //Private variables are shown here. Private methods go after public methods
 private:

  ///Lattice right-side vocabulary  (words)
  unordered_set<std::string> vocabulary_;

  ///Substring version of translation lattice
  fst::VectorFst<Arc> *referencesubstringlattice_;

  ///Full translation lattice
  fst::VectorFst<Arc> *referencelattice_;

  ///Built or not
  bool built_;

  ///Translation lattice file name (to read from) and reference lattice file name (to write to)
  ucam::util::IntegerPatternAddress translationlatticefile_,
       writereferencelatticefile_;

  std::string translationlatticefilesemiring_;
  std::string semiring_;

  std::string oldfile_;

  ///Weight to prune reference lattice with
  float weight_;
  ///Number of paths to shortest path the ref lattice
  unsigned shortestpath_;
  ///To shortestpath/prune or not
  bool useshortestpath_, useweight_;
  ///Disable substring reference
  bool disablesubstring_;

  ///Key -- for storing reference lattice in data object
  const std::string referencelatticekey_;

 public:
  /**
   * \brief Constructor
   * \param rg                            RegistryPO object containing parsed command-line/config-file variables.
   * \param referencelatticekey           key to access the reference lattice file name in registry object (for storing)
   */

  ReferenceFilterTask ( const ucam::util::RegistryPO& rg ,
                        const std::string& referencelatticekey =
                          HifstConstants::kReferencefilterNosubstringStore ) :
    referencelatticekey_ ( referencelatticekey ),
    built_ ( false ),
    translationlatticefile_ ( rg.get<std::string>
                              ( HifstConstants::kReferencefilterLoad ) ),
    translationlatticefilesemiring_ ( rg.get<std::string>
                                      ( HifstConstants::kReferencefilterLoadSemiring ) ),
    semiring_ ( rg.get<std::string>
               ( HifstConstants::kHifstSemiring ) ),
    writereferencelatticefile_ ( rg.get<std::string>
                                 ( HifstConstants::kReferencefilterWrite ) ),
    disablesubstring_ ( rg.getBool ( HifstConstants::kReferencefilterSubstring ) ==
                        false ),
    weight_ ( rg.get<float>
              ( HifstConstants::kReferencefilterPrunereferenceweight ) ),
    shortestpath_ ( rg.get<unsigned>
                    ( HifstConstants::kReferencefilterPrunereferenceshortestpath ) ),
    useshortestpath_ ( rg.get<unsigned>
                       ( HifstConstants::kReferencefilterPrunereferenceshortestpath ) <
                       std::numeric_limits<unsigned>::max() ),
    useweight_ ( rg.get<float>
                 ( HifstConstants::kReferencefilterPrunereferenceweight ) <
                 std::numeric_limits<float>::max() ),
    referencesubstringlattice_ ( NULL ),
    referencelattice_ (NULL) {
  };

  inline bool getDisableSubString ( void ) {
    return disablesubstring_;
  };
  inline bool getBuilt ( void ) {
    return built_;
  };
  inline float getWeight ( void ) {
    return weight_;
  };
  inline unsigned getShortestPath ( void ) {
    return shortestpath_;
  };
  inline const unordered_set<std::string>&  getVocabulary()  {
    return vocabulary_;
  };
  inline const std::string getTranslationLatticeFile()  {
    return translationlatticefile_();
  };

  ///Destructor
  ~ReferenceFilterTask() {
    unload();
  };

  ///Static constructor, returns NULL if the substring lattice is not needed (e.g. hifst not in alignment mode)
  static ReferenceFilterTask* init ( const ucam::util::RegistryPO& rg ,
                                     const std::string& referenceloadkey = HifstConstants::kReferencefilterLoad,
                                     const std::string& referencelatticekey
                                     = HifstConstants::kReferencefilterNosubstringStore ) {
    if ( rg.exists ( referenceloadkey ) )
      if ( rg.get<std::string> ( referenceloadkey ) != "" ) return new
            ReferenceFilterTask ( rg, referencelatticekey );
    return NULL;
  }

  /**
   * \brief Filters the reference lattice using either shortestpath, weighted determinization or both (union).
   */

  void prune() {
    fst::VectorFst<Arc> pruned, dweight;
    if ( useshortestpath_ ) {
      LINFO ( "Using shortestpath with reference lattice n=" << shortestpath_ );
      fst::ShortestPath<Arc> ( *referencesubstringlattice_, &pruned, shortestpath_,
                               true );
    }
    if ( useweight_ ) {
      LINFO ( "Pruning reference lattice with weight=" << weight_ );
      fst::MakeWeight<Arc> mw;
      fst::Prune<Arc> ( referencesubstringlattice_, mw ( weight_ ) );
      LINFO ( "Weighted determinization with weight=" << weight_ );
      fst::DeterminizeOptions<Arc> dopts;
      dopts.weight_threshold = mw ( weight_ );
      fst::Determinize<Arc> ( *referencesubstringlattice_, &dweight, dopts );
    }
    if ( useshortestpath_ || useweight_ )
      *referencesubstringlattice_ = ( fst::UnionFst<Arc> ( pruned,
                                      dweight ) ); //should work either way
  };

  /**
   * \brief Removes weights and reduces the reference lattice with determinization and minimization.
   */

  void reduce() {
    fst::Map<Arc> ( referencesubstringlattice_,
                    fst::RmWeightMapper<Arc>() ); //finally take weights away, so composition scores not affected.
    fst::Determinize<Arc> ( fst::RmEpsilonFst<Arc> ( *referencesubstringlattice_ ),
                            referencesubstringlattice_ );
    fst::Minimize<Arc> ( referencesubstringlattice_ );
  }

  /**
   * \brief Given an fst file, builds the unweighted substring transducer.
   * \remark The lattice can be previously shortestpath-ed or pruned. It will be determinized too
   * \param file: Lattice file name (openfst file expected).
   */
  void build ( const std::string& file ) {
    if ( file == "" ) return;
    if ( built_ && oldfile_ == file ) return;
    oldfile_ = file;
    unload();
    vocabulary_.clear();
    loadLattice(file);
    prune();
    reduce();

    referencelattice_ = new fst::VectorFst<Arc> ( *referencesubstringlattice_ );

    if ( !disablesubstring_ ) {
      LINFO ( "building substring reference" );
      fst::buildSubstringTransducer<Arc>
      ( referencesubstringlattice_ ); //now we build a sstransducer
    } else {
      LWARN ( "Using lattice as-is... substring will not be implemented!!!" );
    }
    fst::ArcSort<Arc> ( referencesubstringlattice_, fst::ILabelCompare<Arc>() );
    fst::extractTargetVocabulary<Arc> ( *referencesubstringlattice_, &vocabulary_ );
    built_ = true;
  };

  ///Clean up fsts...
  void unload ( void ) {
    if ( referencesubstringlattice_ ) delete referencesubstringlattice_;
    referencesubstringlattice_ = NULL;
    built_ = false;
    if ( referencelattice_ ) delete referencelattice_;
    referencelattice_ = NULL;
  }

  ///Write reference substring lattice to [file]
  void write ( Data& d ) {
    if ( writereferencelatticefile_ ( d.sidx ) != "" )
      fst::FstWrite ( *referencesubstringlattice_,
                      writereferencelatticefile_ ( d.sidx ) );
  };

  ///Runs... Load substring lattice and add pointer in data object
  bool run ( Data& d ) {
    LINFO ( "build reference filter from lattice=" << translationlatticefile_.get (
              d.sidx ) );
    build ( translationlatticefile_.get ( d.sidx ) );
    if ( referencesubstringlattice_ ) {
      d.filters.push_back ( referencesubstringlattice_ );
      d.tvcb = vocabulary_;
      d.fsts[referencelatticekey_] = referencelattice_;
      LINFO ( "Done! Full lattice stored with key="
              << referencelatticekey_
              << ", NS=" <<  static_cast<fst::VectorFst<Arc> *>
              ( d.fsts[referencelatticekey_])->NumStates() );
    }
    write ( d );
    return false;
  };

 private:

  void loadLattice(std::string const &file) {
    using namespace fst;
    if (translationlatticefilesemiring_ == "" ) { // use default arc
      referencesubstringlattice_ = VectorFstRead<Arc> ( file );
      return;
    }
    if (semiring_ != "tuplearc") {
      LERROR("Conversions currently allowed only from lexstdarc,tropical TO tuplearc)");
      exit(EXIT_FAILURE);
    }
    referencesubstringlattice_ = new VectorFst<Arc>;

    if (translationlatticefilesemiring_ == "lexstdarc") {
      VectorFst<LexStdArc> *aux= VectorFstRead<LexStdArc> ( file );
      VectorFst<TupleArc32> *vwfst = new VectorFst<TupleArc32>;

      LINFO ( "Mapping Arc Target Lattice to TupleArc32" );
      MakeSparseVectorWeight<LexStdArc> mwcopy ( 1 );
      typedef GenericWeightMapper<LexStdArc, TupleArc32, MakeSparseVectorWeight<LexStdArc> > WeightMapper;
      Map ( *aux, vwfst, WeightMapper(mwcopy));
      // bypassing template instance conversions with a reinterpret cast
      // but this code only happens from tuplearc32 to tuplearc32.
      referencesubstringlattice_ = reinterpret_cast<VectorFst<Arc> *>(vwfst);

      delete aux;
      return;
    }
  }


  ZDISALLOW_COPY_AND_ASSIGN ( ReferenceFilterTask );

};

}
} // end namespaces

#endif // TASK_REFERENCE_FST_HPP
