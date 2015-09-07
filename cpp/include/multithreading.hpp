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

#ifndef MULTITHREADING_HPP
#define MULTITHREADING_HPP

/**
 * \file
 * \brief Implements trivial threadpool using boost::asio library
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace util {
/**
 * \brief Trivial implementation of a threadpool based on boost::asio methods
 * When initiated, creates a threadpool of n threads (n <= number of cpus).
 * Jobs should be submitted with the templated operator(). When the object is deleted
 * it will wait for all threads to finish.
 */

class TrivialThreadPool {

 private:
  boost::thread_group pool_;
  boost::asio::io_service service_;
  boost::scoped_ptr<boost::asio::io_service::work> work_;
  std::size_t numthreads_;

 public:
  TrivialThreadPool ( std::size_t n )
    : numthreads_( n > boost::thread::hardware_concurrency()
                   ? boost::thread::hardware_concurrency() : n )
    , service_ ( numthreads_ )
    , work_ ( new boost::asio::io_service::work ( service_ ) )
  {
    USER_CHECK ( numthreads_ > 0
                 , "Number of threads has to be greater than 0!" );
    for ( std::size_t i = 0; i < numthreads_; i++ )
      pool_.create_thread ( boost::bind ( &boost::asio::io_service::run,
                                          &service_ ) );
  }

  ~TrivialThreadPool() {
    work_.reset();
    pool_.join_all();
  }

  template<typename F>
  void operator() ( F task ) {
    service_.post ( task );
  }

};

/**
 * \brief Trivial struct that can
 * replace seamlessly the threadpool
 * for single threaded executions
 */
 struct NoThreadPool {
  template<typename F>
  void operator() (F &task) {
    task();
  }
};


}
} // end namespaces

#endif
