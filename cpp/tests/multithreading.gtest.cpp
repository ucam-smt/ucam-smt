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
 * \brief Unit testing: multithreading with a threadpool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "multithreading.hpp"

extern bool user_check_ok;

namespace googletesting {

struct functor_test {
  unsigned silly_;
  functor_test ( unsigned a = 0 ) : silly_ ( a ) {};
  void operator() () {
    silly_ *= 10;
  };
};

struct functor_test2 {
  unsigned *silly_;
  functor_test2 ( unsigned a ) : silly_ ( new unsigned ( a ) ) {
    LINFO ("Hello " << uu::toString (*silly_) );
  };
  void operator() () {
    *silly_ *= 10;
    LINFO ( "silly=" << *silly_ );
  };
};

};

/// This test shows that functors are actually being passed by value to the threadpool, not per reference.
/// First time i use these cool asio libraries, so I might be missing something here, but
/// it initially looks like a limitation of asio::ioservice::post method (cannot pass by reference)

TEST ( multithreading, threadpool1 ) {
  googletesting::functor_test a1 ( 1 ), a2 ( 2 ), a3 ( 3 ), a4 ( 4 );
  {
    uu::TrivialThreadPool tp ( 4 );
    tp ( a1 );
    tp ( a2 );
    tp ( a3 );
    tp ( a4 );
  }
  EXPECT_EQ ( a1.silly_, 1 );
  EXPECT_EQ ( a2.silly_, 2 );
  EXPECT_EQ ( a3.silly_, 3 );
  EXPECT_EQ ( a4.silly_, 4 );
}

/// This test should actually work well.
///Passing these functors by value will bit-copy the pointer addresses, and pointees will be modified.
TEST ( multithreading, threadpool2 ) {
  googletesting::functor_test2 a1 ( 1 );
  googletesting::functor_test2 a2 ( 2 );
  googletesting::functor_test2 a3 ( 3 );
  googletesting::functor_test2 a4 ( 4 );
  {
    uu::TrivialThreadPool tp ( 4 );
    tp ( a1 );
    tp ( a2 );
    tp ( a3 );
    tp ( a4 );
  }
  ///Safe to test
  EXPECT_EQ ( *a1.silly_, 10 );
  EXPECT_EQ ( *a2.silly_, 20 );
  EXPECT_EQ ( *a3.silly_, 30 );
  EXPECT_EQ ( *a4.silly_, 40 );
  //Extra careful memory management...
  delete a1.silly_;
  delete a2.silly_;
  delete a3.silly_;
  delete a4.silly_;
  user_check_ok = true;
  ///This should stop execution:
  {
    uu::TrivialThreadPool tp ( 0 );
  }
  EXPECT_EQ ( user_check_ok, false );
  user_check_ok = true;
}

#ifndef GMAINTEST

/**
 * \brief main function.
 * If compiled individualy, will kickoff any tests in this file.
 */
int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
#endif
