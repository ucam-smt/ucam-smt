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

#ifndef TASKDATA_HPP
#define TASKDATA_HPP

/** \file
 *\brief Data object for hifst or related tools.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 *\brief Data class containing relevant variables. To be used as template for task classes using it.
 *
 */
template <class ArcT = fst::LexicographicArc< fst::StdArc::Weight, fst::StdArc::Weight> >
class HifstTaskData {
  typedef ucam::util::WordMapper WordMapper;
  typedef ucam::fsttools::StatsData StatsData;
  typedef typename ucam::fsttools::KenLMData KenLMData;

 public:
  HifstTaskData() :
    sidx ( 0 ),
    grammar ( NULL ),
    ssgd ( NULL ),
    cykdata ( NULL ),
    numlocallm ( 0 ),
    stats ( new StatsData ),
    translation ( NULL ) {
  };

  /// Sentence index
  unsigned sidx;
  ///Contains translation grammar
  const GrammarData *grammar;

  ///Contains oovs
  unordered_map<std::size_t, std::string> oovwmap;

  ///source sentence
  std::string originalsentence;
  std::string tokenizedsentence;
  std::string sentence;

  ///Pattern instances
  std::vector<std::string> pinstances;

  /// Holds instanced patterns (string) over the sentence, mapped to extra information pair<1,2>:
  /// positions at which these were encountered (1), and minimum span (2).
  /// \todo Possibly, add minimum span
  unordered_map<std::string, std::vector< pair <unsigned, unsigned> > > hpinstances;

  ///Sentence-specific grammar information -- hashes to rule indices.
  SentenceSpecificGrammarData *ssgd;

  ///Target vocabulary
  unordered_set<std::string> tvcb;

  ///cyk data structures
  CYKdata *cykdata;

  //Filters, e.g. translation lattice substring for alignment or others
  //\todo delete and add in fsts ?
  std::vector< fst::VectorFst<ArcT> *> filters;

  ///Pointers to lattices (e.g. translation lattice, lmbr, etc) , and related, accessed by unique keys
  //  unordered_map<string, fst::VectorFst<ArcT> * > fsts;
  unordered_map<std::string, void * > fsts;

  inline fst::VectorFst<ArcT> *getFst(std::string const &key) {
    unordered_map<std::string, void * >::iterator itx
        = fsts.find ( key);
    if ( itx == fsts.end() ) {
      LWARN("Empty lattice key=" << key);
      return NULL;
    }
    return reinterpret_cast<fst::VectorFst<ArcT> *>(itx->second);
  }



  ///Collections of language models accessed by keys (e.g. in translation we need a bunch for hifst and one for recaser)
  unordered_map<std::string, std::vector <const KenLMData *> > klm;
  ///Number of local language models used in hifst
  unsigned numlocallm;

  ///To collect statistics across the whole pipeline.
  boost::shared_ptr<StatsData> stats;

  ///Translated sentence will be stored here
  std::string *translation;

  ///mixed-case vocabulary of the recasing unigram language model
  unordered_set<std::string> *recasingvcblm;

  /// This information used for stats
  grammar_inversecategories_t vcat;

  ///Wordmap/Integer map objects
  unordered_map<std::string, WordMapper *> wm;

};



}} // end namespaces

#endif

