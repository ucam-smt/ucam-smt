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

#ifndef TASK_RULES2FLOWERFST_HPP
#define TASK_RULES2FLOWERFST_HPP

/**
 * \file
 * \brief Implements a class that loads the grammar sparseweight flower lattice and stores a pointer on the data object
 * \date 10-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/// Implements a class that loads the grammar sparseweight flower lattice and stores a pointer on the data object
template<class Data>
class LoadSparseWeightFlowerLatticeTask: public
  ucam::util::TaskInterface<Data> {
 private:

  ///Fst with the flower lattice itself
  fst::VectorFst<TupleArc32> flowerlattice_;

  ///sparse tuple-weight scales
  std::vector<float>& fscales_;

  ///Alignment lattices file names
  ucam::util::IntegerPatternAddress alilats_;

  ///Grammar file name
  ucam::util::IntegerPatternAddress grammar_;

  ///Previous grammar file name
  std::string previousfile_;

  ///If true, the grammar flower lattice will be loaded only with rules that have been used in the alignment lattices
  bool filterbyalilats_;

  ///Registry object -- contains program options
  const ucam::util::RegistryPO& rg_;

  /// Is the flower lattice built
  bool built_;

  ///Key with access to registry object for the grammar file name
  const std::string grammarloadkey_;
  ///Key to store in the data object
  const std::string grammarstorekey_;

  ///Number of language models
  const unsigned offset_;
 public:
  ///Constructor with registry object, offset and keys
  LoadSparseWeightFlowerLatticeTask ( const ucam::util::RegistryPO& rg,
                                      const unsigned offset =
                                        1, //minimum offset considering only one language model...
                                      const std::string& alignmentlattices =
                                        HifstConstants::kRuleflowerlatticeFilterbyalilats,
                                      const std::string& grammarloadkey = HifstConstants::kRuleflowerlatticeLoad,
                                      const std::string& grammarstorekey = HifstConstants::kRuleflowerlatticeStore
                                    ) :
    offset_ ( offset ),
    rg_ ( rg ),
    alilats_ ( rg.exists ( alignmentlattices ) ? rg.get<std::string>
               ( alignmentlattices ) : "" ),
    grammar_ ( rg.get<std::string> ( grammarloadkey ) ),
    fscales_ ( fst::TropicalSparseTupleWeight<float>::Params() ),
    filterbyalilats_ ( rg.exists ( alignmentlattices ) ),
    grammarstorekey_ ( grammarstorekey ) {
  };

  ///Inherited method from ucam::util::TaskInterface. Loads the flower lattice into the data object.
  bool run ( Data& d ) {
    load ( grammar_ ( d.sidx ) );
    d.fsts[grammarstorekey_] = &flowerlattice_;
    return false;
  };

  ///If it is an fst, load directly
  bool directload ( const std::string& filename ) {
    bool directload = false;
    std::vector<std::string> vgn;
    boost::algorithm::split ( vgn, filename, boost::algorithm::is_any_of ( "." ) );
    if ( vgn[vgn.size() - 1] == "fst" ) directload = true;
    else if ( vgn.size() > 2 ) if ( vgn[vgn.size() - 1] == "gz"
                                      && vgn[vgn.size() - 2] == "fst" ) directload = true;
    if ( directload ) {
      LINFO ( "Loading FST directly (assumes arcsorted flower) ="  << filename );
      fst::VectorFst<TupleArc32> *yupi = fst::VectorFstRead<TupleArc32> ( filename );
      flowerlattice_ = *yupi;
      delete yupi;
      return true;
    }
    return false;
  }

  /**
   * \brief Load flower lattice from file.
   * \remark Each rule will be a single arc in the fst with the rule index
   * Tropical Sparse Tuple weight is used. Each arc (rule) has a set of feature weights.
   * The first weight is at offset_+1. Weight indices 1..offset_ are left empty for the language model(s)
   * to fill in.
   */
  bool load ( const std::string& filename ) {
    if ( !USER_CHECK ( filename != "" , "No grammar to load?" ) ) return false;
    if ( filename == previousfile_ )  {
      LINFO ( "Skipping grammar loading..." );
      return false;
    }
    previousfile_ = filename;
    FORCELINFO ( "building rule flower from " << filename );
    if ( directload ( filename ) ) return true;
    ///Read alilats and get rules
    //    unordered_set<TupleArc32::Label> idxrules;
    unordered_set<unsigned> idxrules;
    if ( filterbyalilats_ )
      for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ) );
            !ir->done ();
            ir->next () ) {
        fst::VectorFst<fst::LexStdArc> *alilatsfst =
          fst::VectorFstRead<fst::LexStdArc> ( alilats_ ( ir->get() ) );
        fst::extractSourceVocabulary<fst::LexStdArc> ( *alilatsfst, &idxrules );
        delete alilatsfst;
      }
    flowerlattice_.AddState();
    flowerlattice_.SetStart ( 0 );
    flowerlattice_.SetFinal ( 0, TupleArc32::Weight::One() );
    unsigned lc = 0, llc = 0;
    ucam::util::iszfstream myrulefile;
    myrulefile.open ( filename.c_str() );
    LINFO ( "Opening rule file " << filename );
    if ( !myrulefile.is_open() ) {
      LERROR ( "Failed to open " << filename );
      return false;
    }
    do {
      std::string line;
      getline ( myrulefile, line );
      //  LDEBUG2(line);
      ++lc;
      if ( line.size() > 0 ) {
        while ( line.at ( line.length() - 1 ) == ' ' )
          line.erase ( line.length() - 1 );
        while ( line.at ( 0 ) == ' ' )
          line.erase ( 0, 1 );
        if ( line.at ( 0 ) == '#' || line.at ( 0 ) == '%' ) line = "";
      }
      if ( line == "" ) {
        LDEBUG ( lc << "is an empty line/comment." );
        continue;
      }
      if ( ! ( lc % 100000 ) ) LINFO ( lc << " rules parsed..." );
      if ( !idxrules.empty() )
        if ( idxrules.find ( lc ) == idxrules.end() ) continue;
      llc++;
      TupleW32 vtcost;
      std::vector<std::string> fields;
      boost::algorithm::split ( fields, line, boost::algorithm::is_any_of ( " " ) );
      for ( unsigned k = 3; k < fields.size(); ++k ) {
        float prob;
        unsigned pos = 0;
        std::vector<std::string> splitfields;
        boost::algorithm::split ( splitfields, fields[k],
                                  boost::algorithm::is_any_of ( "@" ) );
        using ucam::util::toNumber;
        if ( splitfields.size() == 2 ) {
          if ( splitfields[0] == "" ) prob = 1.000f;
          else prob = toNumber<float> ( splitfields[0] );
          pos = offset_ + toNumber<unsigned> ( splitfields[1] );
        } else {
          prob = toNumber<float> ( fields[k] );
          pos = offset_ + k - 2;
          LDEBUG ( pos );
        }
        vtcost.Push ( pos,
                      prob ); // It's sparse weight -- will take care of taking out zeros.
        splitfields.clear();
      }
      unsigned label = lc;
      flowerlattice_.AddArc ( 0, TupleArc32 ( label, label, vtcost, 0 ) );
    } while ( myrulefile && !myrulefile.eof() );
    LINFO ( "File: " << filename << " has been succesfully loaded" );
    myrulefile.close();
    LINFO ( "Number of rules actually loaded for this job: " << llc );
    fst::ArcSort<TupleArc32> ( &flowerlattice_, fst::ILabelCompare<TupleArc32>() );
    return built_ = true;
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( LoadSparseWeightFlowerLatticeTask );
};

}
} // end namespaces

#endif // TASK_RULES2FLOWERFST_HPP
