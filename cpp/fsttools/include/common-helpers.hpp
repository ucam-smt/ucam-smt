#pragma once

#include <registrypo.hpp>
#include <kenlmdetect.hpp>


namespace ucam {
namespace fsttools {

/**
 * @brief Generic Runner2 class wrapper with the usual template structure
 * required by the tasks in fsttools and hifst. This one is meant to be
 * used by most of the tools.
 */
template <
  template <template <class, class> class
            , class
            , class> class TaskOneT
  , template <template <class, class> class
              , class
              , class> class TaskTwoT
  , template <class, class> class DataT
  , class KenLMModelT
  , class ArcT
  >
struct RunTask2 {
  explicit RunTask2(util::RegistryPO const &rg){
  using util::Runner2;
  ( Runner2<
    TaskOneT< DataT, KenLMModelT, ArcT>
    , TaskTwoT< DataT, KenLMModelT, ArcT>
    >
    ( rg ) ) ();
  }
};


/**
 * @brief Generic Runner3 class wrapper with the usual template structure
 * required by hifst. It can be used by other tools that should support
 * a server mode (see Runner2/Runner3 details).
 */
template <
  template <template <class, class> class
            , class
            , class> class TaskOneT
  , template <template <class, class> class
              , class
              , class> class TaskTwoT
  , template <template <class, class> class
              , class
              , class> class TaskThreeT
  , template <class, class> class DataT
  , class KenLMModelT
  , class ArcT
  >
struct RunTask3 {
  explicit RunTask3(util::RegistryPO const &rg){
  using util::Runner3;
  ( Runner3<
    TaskOneT< DataT, KenLMModelT, ArcT>
    , TaskTwoT< DataT, KenLMModelT, ArcT>
    , TaskThreeT< DataT, KenLMModelT, ArcT>
    >
    ( rg ) ) ();
  }
};



#ifdef KENLM_MAX_ORDER
/**
 * @brief Generic Kenlm-template-instancing wrapper
 *
 * KenLM can autodetect the appropriate templates to be used
 * based on the file it is loading.
 * This templated function is expected to be used by all tools
 * in ucam package. In this way, it will be easy to ensure that all tools
 * that need to apply the language models can take advantage automatically
 * of all available formats, by just adding here new cases in the switch.
 * It relies on.
 */
template< template < template <class, class> class
                     , class
                     , class> class RunTaskT
          , template <class, class> class DataT
          , class ArcT
          >
inline void runTaskWithKenLMTemplate(util::RegistryPO const &rg) {
  using namespace lm::ngram;
  typedef lm::np::Model NplmModel;
  // Detect here kenlm binary type
  // it's a bit ugly this way of initializing the correct kenlm handler
  //  ModelType kenmt = util::detectkenlm
  int  kenmt = util::detectkenlm
      (rg.getVectorString (HifstConstants::kLmLoad, 0) );

  switch (kenmt) {
  case PROBING:
    (RunTaskT<DataT, ProbingModel, ArcT>(rg));
    break;
  case REST_PROBING:
    (RunTaskT<DataT, RestProbingModel, ArcT>(rg));
    break;
  case TRIE:
    (RunTaskT<DataT, TrieModel, ArcT>(rg));
    break;
  case QUANT_TRIE:
    (RunTaskT<DataT, QuantTrieModel, ArcT>(rg));
    break;
  case ARRAY_TRIE:
    (RunTaskT<DataT, ArrayTrieModel, ArcT>(rg));
    break;
  case QUANT_ARRAY_TRIE:
    (RunTaskT<DataT, QuantArrayTrieModel, ArcT>(rg));
    break;
//   case util::MULTIPLE_LMS:
//     (RunTaskT<DataT, lm::base::Model , ArcT>(rg));
//     break;
      // not tested yet:
  case util::KENLM_NPLM:
#ifdef WITH_NPLM
    (RunTaskT<DataT, NplmModel, ArcT>(rg));
    break;
#endif
    std::cerr << "Unsuported format: KENLM_NPLM. Did you compile NPLM library?" << std::endl;
    exit(EXIT_FAILURE);
  }
};
#endif

}}  // end namespaces
