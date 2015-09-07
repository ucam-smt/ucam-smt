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

#ifndef MULTITHREADING_HELPERS_HPP
#define MULTITHREADING_HELPERS_HPP

#include <multithreading.hpp>

namespace HifstConstants {
const std::string kNThreads = "nthreads";
const std::string kServerEnable = "server.enable";
}

namespace ucam {
namespace util {

/**
 * \brief Convenience wrapper class that can kick off two type of executions: singlethreaded or multithreaded, triggered by program options.
 * Possibly multithreading with 1 thread would do, but I keep both implementations as any plain bug that might arise will be easier
 * to trace down with a normal execution (threadpool uses two, actually).
 * The class is templated with two classes, one for single threading and another for multithreading.
 * Note that the multithreading details are up to the second templated class.
 * e.g. Runner<SingleThreadedFunctor,SingleThreadedFunctor> would not multithread at all ;-).
 */

template<class SingleThreadedFunctorT, class MultiThreadedFunctorT>
class Runner {
 private:
  //Use multithreading or not
  bool multithread_;
  //How many threads is the user requesting
  unsigned threadcount_;
  //Command-line/config file options
  const RegistryPO& rg_;

  SingleThreadedFunctorT sf_;
  MultiThreadedFunctorT mtf_;

 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  Runner ( const RegistryPO& rg ) :
    rg_ ( rg ),
    multithread_ ( rg.exists ( HifstConstants::kNThreads ) ),
    threadcount_ ( multithread_ ? rg.get<unsigned> ( HifstConstants::kNThreads ) :
                   0 ) {
    if ( multithread_ ) {
      LINFO ( "Multithreading:" << threadcount_ << " threads" );
    } else {
      LINFO ( "Multithreading: no" );
    }
  };

  ///Either runs with multithreading or single-thread option.
  inline void operator() () {
    if ( multithread_ ) mtf_ ( rg_, threadcount_ );
    else sf_ ( rg_ );
  };

};

/**
 * \brief Convenience wrapper class that can kick off two type of executions: single or multithreaded, triggered by program options.
 * Possibly multithreading with 1 thread would do, but I keep both implementations as any plain bug that might arise will be easier
 * to trace down with a serialized execution (threadpool uses two, actually).
 * The class is templated with two classes, one for single threading and another for multithreading.
 * Note that the multithreading details are up to the second templated class.
 */

template<class SingleThreadedFunctorT, class MultiThreadedFunctorT >
class Runner2 {
 private:
  //Use multithreading or not
  bool multithread_;
  //Command-line/config file options
  const RegistryPO& rg_;

 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  Runner2 ( const RegistryPO& rg ) :
    rg_ ( rg ),
    multithread_ ( rg.exists ( HifstConstants::kNThreads ) )

  {
    if ( multithread_ ) {
      LINFO ( "Multithreading:yes" );
    } else {
      LINFO ( "Multithreading:no" );
    }
  };

  ///Either runs with multithreading or single-thread option.
  inline void operator() () {
    if ( multithread_ ) {
      MultiThreadedFunctorT mtf ( rg_ );
      mtf();
      return;
    }
    SingleThreadedFunctorT sf ( rg_ );
    sf();
  };

};

/**
 * \brief Convenience wrapper class that can kick off three type of executions: singlethreaded, multithreaded, or server, triggered by program options.
 * Possibly multithreading with 1 thread would do, but I keep both implementations as any plain bug that might arise will be easier
 * to trace down with a single thread execution.
 * The class is templated three functors, one for each type of execution
 * Note that the details are up to the each of these functors
 */

template<class SingleThreadedFunctorT, class MultiThreadedFunctorT, class ServerFunctorT >
class Runner3 {
 private:
  //Use multithreading or not
  bool multithread_;
  //Command-line/config file options
  const RegistryPO& rg_;

  bool server_;

 public:
  /**
   * \brief Constructor
   * \param rg: Registry object containing parameters
   */
  Runner3 ( const RegistryPO& rg ) :
    rg_ ( rg ),
    multithread_ ( rg.exists ( HifstConstants::kNThreads ) ),
    server_ ( rg.getBool ( HifstConstants::kServerEnable ) )

  {
    if ( server_ ) {
      LINFO ( "Initializing server..." );
    } else if ( multithread_ ) {
      LINFO ( "Multithreading:yes" );
    } else {
      LINFO ( "Multithreading:no" );
    }
  };

  ///Either runs with multithreading or single-thread option.
  inline void operator() () {
    if ( server_ ) {
      ServerFunctorT serv ( rg_ );
      serv();
      return;
    }
    if ( multithread_ ) {
      MultiThreadedFunctorT mtf ( rg_ );
      mtf();
      return;
    }
    SingleThreadedFunctorT sf ( rg_ );
    sf();
  };

};

}
} // end namespaces
#endif
