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

#ifndef TEXTOUTPUTTASK_HPP
#define TEXTOUTPUTTASK_HPP

/** \file
 *  \brief Task that writes translation to a text file. This translation might be wordmapped and tokenized.
 * \date 20-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 * \brief Task that writes translation to a text file. This translation might be recased, wordmapped and tokenized.
 */

template <class Data, class Arc = fst::StdArc>
class PostProTask: public ucam::util::TaskInterface<Data> {

  //Private variables are shown here. Private methods go after public methods
 private:

  /// wordmap class.
  ucam::util::WordMapper *trgidx2wmap_;

  const std::string wordmapkey_;

  /// Use wordmap or not.
  bool usewordmap_;

  ///Detokenize or not (only if usewordmap_ activated too)
  bool detokenize_;

  /// Language for detokenization. Only specify for French
  std::string detokenizationlanguage_;

  const std::string inputkey_;

  //Capitalize systematically first word.
  bool capitalizeFirstWord_;

 public:
  ///Constructor with ucam::util::RegistryPO object and keys to access lattice and wordmap
  PostProTask ( const ucam::util::RegistryPO& rg ,
                const std::string& inputkey = HifstConstants::kPostproInput,
                const std::string& wordmapkey = HifstConstants::kPostproWordmapLoad
              )
    : inputkey_ ( inputkey )
    , wordmapkey_ ( wordmapkey )
    , trgidx2wmap_ ( NULL )
    , detokenize_ ( rg.getBool ( HifstConstants::kPostproDetokenizeEnable ) )
    , detokenizationlanguage_ ( rg.exists (
                                  HifstConstants::kPostproDetokenizeLanguage) ? rg.get<std::string>
                                (HifstConstants::kPostproDetokenizeLanguage) : "")
    , capitalizeFirstWord_ (rg.getBool (
                              HifstConstants::kPostproCapitalizefirstwordEnable) )

  {
    LDEBUG ( "Constructor ready..." );
  };

  ///Turn on/off tokenization
  inline void setDetokenize ( bool detok ) {
    detokenize_ = detok;
  };

  /**
   * \brief Writes 1-best to file. Optionally, recases, maps back to words, and detokenizes.
   * \param &d: Data structure conatining d.translationlattice.
   */

  bool run ( Data& d ) {
    if ( !USER_CHECK ( d.fsts[inputkey_] != NULL,
                       "translation lattice not initialized?" ) ) return true;
    if ( !USER_CHECK ( d.translation != NULL,
                       "d.translation not initialized?" ) ) return true;
    fst::VectorFst<Arc> ofst ( * (static_cast< fst::VectorFst<Arc> *>
                                  (d.fsts[inputkey_]) ) );
    std::string text;
		fst::FstGetBestStringHypothesis( ofst, text);
    LINFO ( "1best is " << text );
    std::string detokutext;
    if ( d.wm.find ( wordmapkey_ ) != d.wm.end() )
      trgidx2wmap_ = d.wm[wordmapkey_];
    else trgidx2wmap_ = NULL;
    if ( trgidx2wmap_ ) {
      std::string utext;
      trgidx2wmap_->set_oovwmap ( d.oovwmap );
      ( *trgidx2wmap_ ) ( text, &utext , false );
      LINFO ( "(unmapped) 1best is:" << utext );
      //Take out 1 and 2 if they exist
      ucam::util::deleteSentenceMarkers ( utext );
      if ( detokenize_ ) {
        ucam::util::detokenize ( utext, &detokutext , detokenizationlanguage_ );
        LINFO ( "1best (detok) is:" << detokutext );
      } else detokutext = utext;
      if ( capitalizeFirstWord_ ) {
        ucam::util::capitalizeFirstWord ( detokutext );
      }
    } else detokutext = text;
    FORCELINFO ( "Translation 1best is: " << detokutext );
    *d.translation = detokutext;
    return false;
  };

 private:

  ZDISALLOW_COPY_AND_ASSIGN ( PostProTask );

};

}
}  // end namespaces

#endif
