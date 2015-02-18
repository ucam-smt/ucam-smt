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
  template <template <class> class
            , class> class TaskOneT
  , template <template <class> class
              , class> class TaskTwoT
  , template <class> class DataT
  , class ArcT
  >
struct RunTask2 {
  explicit RunTask2(util::RegistryPO const &rg){
  using util::Runner2;
  ( Runner2<
    TaskOneT< DataT, ArcT>
    , TaskTwoT< DataT, ArcT>
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
  template <template <class> class
            , class> class TaskOneT
  , template <template <class> class
              , class> class TaskTwoT
  , template <template <class> class
              , class> class TaskThreeT
  , template <class> class DataT
  , class ArcT
  >
struct RunTask3 {
  explicit RunTask3(util::RegistryPO const &rg){
  using util::Runner3;
  ( Runner3<
    TaskOneT< DataT, ArcT>
    , TaskTwoT< DataT, ArcT>
    , TaskThreeT< DataT, ArcT>
    >
    ( rg ) ) ();
  }
};

}}  // end namespaces
