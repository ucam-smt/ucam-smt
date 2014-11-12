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

#ifndef TASK_HIFST_REPLACEFSTBYARC_HPP
#define TASK_HIFST_REPLACEFSTBYARC_HPP

/**
 * \file
 * \brief Contains Function objects that determine whether an FST is replaceable or not by an arc pointer
 */

namespace ucam {
namespace hifst {

template<class Arc>
struct GenerateTrivialFst {
  bool aligner_;
  explicit GenerateTrivialFst(bool alignmode)
      : aligner_(alignmode)
  {};

  fst::VectorFst<Arc> *operator()(typename Arc::Label const& hieroindex) const {
    fst::VectorFst<Arc>* outfst = new fst::VectorFst<Arc>;
    outfst->AddState();
    outfst->SetStart ( 0 );
    outfst->AddState();
    outfst->SetFinal ( 1, Arc::Weight::One() );
    if ( aligner_ )
      outfst->AddArc ( 0, Arc ( NORULE, hieroindex, Arc::Weight::One() , 1 ) );
    else
      outfst->AddArc ( 0, Arc ( hieroindex, hieroindex, Arc::Weight::One() , 1 ) );
    return outfst;
  };
};


/**
 * \brief Creates FST replacement or not depending on conditions.
 * \remark The criterion in this case is simply the number of states. If It is higher than a threshold,
 * then it is replaceable and it will generate the FST replacement (two states binded by pointer arc )
 */
template<class Arc = fst::LexStdArc>
class ReplaceFstByArc {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:
  ///Aligner mode
  //  bool aligner_;
  ///Number of states threshold
  const std::size_t minns_;
  GenerateTrivialFst<Arc> gtf_;
 public:
  ////Constructor accepting two parameters: alignment mode and minimum number of states
  ReplaceFstByArc ( bool alignmode,
                    std::size_t min_numstates = 2 )
      : minns_ ( min_numstates )
      , gtf_(alignmode)
  {};

  /**
   * \brief Determines whether an FST is replaceable
   * If so (NS > minns), then it builds the FST replacement and returns it. Otherwise returns NULL.
   * \param fst                The fst candidate for replacing with a pointer arc
   * \param hieroindex         Index for cc,x,y in the cyk grid cell
   */
  inline fst::VectorFst<Arc> *operator() ( fst::VectorFst<Arc> const& fst,
      Label const& hieroindex ) const {
    if ( fst.NumStates() <= minns_ ) return NULL;
    return gtf_(hieroindex);
  };

 private:
  DISALLOW_COPY_AND_ASSIGN ( ReplaceFstByArc );
};

/**
 * \brief Creates FST replacement or not depending on conditions including program options.
 * \remark In addition to ReplaceFstByArc, this class also considers a list of non-terminals
 * For cell lattices associated to any of the non-terminals in this list, an FST with a pointer arc
 * will be always generated.
 */
template<class Arc = fst::LexStdArc>
class ManualReplaceFstByArc {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:
  /// Alignment mode
  // bool aligner_;
  /// Set of unique non-terminals. Cells representing theses non-terminals will be replaced
  unordered_set<std::string> replacefstbyarc_;
  unordered_set<std::string> replacefstbyarcexceptions_;
  grammar_inversecategories_t vcat_;
  /// Minimum number of states
  std::size_t const minns_;
  GenerateTrivialFst<Arc> gtf_;
 public:
  /**
   * \brief
   *  \param vcat             cc - string category mapping
   * \param replacefstbyarc   Contains a list of non-terminals corresponding to cell lattices that must be replaced
   * \param alignmode      Hifst in alignment mode or not
   * \param min_numstates  Number of states minimum threshold
   */
  ManualReplaceFstByArc ( grammar_inversecategories_t const& vcat,
                          unordered_set<std::string> const& replacefstbyarc,
                          bool alignmode,
                          std::size_t min_numstates = std::numeric_limits<std::size_t>::max()
                        )
      : vcat_ ( vcat )
      , replacefstbyarc_ ( replacefstbyarc )
      , minns_ ( min_numstates )
      , gtf_(alignmode)
  {};

  ManualReplaceFstByArc ( grammar_inversecategories_t const& vcat,
                          unordered_set<std::string> const& replacefstbyarc,
                          unordered_set<std::string> const& replacefstbyarcexceptions,
                          bool alignmode,
                          std::size_t min_numstates = std::numeric_limits<std::size_t>::max()
                          )
      : vcat_ ( vcat )
      , replacefstbyarc_ ( replacefstbyarc )
      , replacefstbyarcexceptions_ ( replacefstbyarcexceptions )
      , minns_ ( min_numstates )
      , gtf_(alignmode)
  {};

  /**
   * \brief Determines whether an FST is replaceable
   * If so (NS > minns or determined by program option hifst.replacfstbyarc), then
   * it builds the FST replacement and returns it. Otherwise returns NULL.
   * \param fst                The fst candidate for replacing with a pointer arc
   * \param hieroindex         Index for cc,x,y in the cyk grid cell
   */

  inline fst::VectorFst<Arc> *operator() ( fst::VectorFst<Arc> const& fst,
      Label const& hieroindex ) const {
    grammar_inversecategories_t::const_iterator itx = vcat_.find ( (
          hieroindex - APBASETAG ) / APCCTAG );
    if ( !USER_CHECK ( itx != vcat_.end(), "Category not identified!" ) ) {
      LERROR ( "Category id is " << ( hieroindex - APBASETAG ) / APCCTAG );
      return NULL;
    }
    if (replacefstbyarcexceptions_.find (itx->second) !=
        replacefstbyarcexceptions_.end() )
      return NULL;
    if ( replacefstbyarc_.find ( itx->second ) != replacefstbyarc_.end()
         || fst.NumStates() >= minns_
       ) {
      return gtf_(hieroindex);
    }
    return NULL;
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( ManualReplaceFstByArc );
};

}
} // end namespaces

#endif
