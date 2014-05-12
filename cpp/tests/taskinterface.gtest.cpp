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

/** \file tests/taskinterface.gtest.cpp
 * \brief Unit testing: TaskInterface methods
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <googletesting.h>

#ifndef GMAINTEST
#include "main.custom_assert.hpp"
#include "main.logger.hpp"
#endif

#include "taskinterface.hpp"

///test-specific classes and functions
namespace googletesting {

///Trivial data class with three bool variables.
class DataTest1 {
 public:
  bool modifiedbytask1;
  bool modifiedbytask2;
  bool modifiedbytask3;
};

///Trivial task class implementing run method to modify first bool variable
template<class Data>
class Task1: public uu::TaskInterface<Data> {
 public:
  bool run ( Data& d ) {
    d.modifiedbytask1 = true;
    return false;
  };

};

///Trivial task class implementing run method to modify second bool variable
template<class Data>
class Task2: public uu::TaskInterface<Data> {
 public:
  bool run ( Data& d ) {
    d.modifiedbytask2 = true;
    return false;
  };
};

///Trivial task class implementing run method to modify third  bool variable
template<class Data>
class Task3: public uu::TaskInterface<Data> {
 public:
  bool run ( Data& d ) {
    d.modifiedbytask3 = true;
    return false;
  };
};

/**
 * \brief Tests chainoftasks implementation in taskinterface. Three objects are appended
 * Each of them modifies a single bool. The three should get executed so all bool variables are set to true.
 */
TEST ( TaskInterface, chainoftasks1 ) {
  Task1<DataTest1> t1;
  t1.appendTask ( new Task2<DataTest1> );
  t1.appendTask ( new Task3<DataTest1> );
  DataTest1 d;
  d.modifiedbytask1 = d.modifiedbytask2 = d.modifiedbytask3 = false;
  t1.chainrun ( d );
  EXPECT_EQ ( d.modifiedbytask1, true );
  EXPECT_EQ ( d.modifiedbytask2, true );
  EXPECT_EQ ( d.modifiedbytask3, true );
}

///Trivial task class implementing run method to modify second bool variable. Additionally, signals that execution is to stop after finishing with Task2b.
template<class Data>
class Task2b: public uu::TaskInterface<Data> {
 public:
  bool run ( Data& d ) {
    d.modifiedbytask2 = true;
    return true;
  };
};

/**
 * \brief Tests chainoftasks implementation in taskinterface. Three objects are appended
 * Each of them modifies a single bool. The three should get executed so all bool variables are set to true.
 */
TEST ( TaskInterface, chainoftasks2 ) {
  Task1<DataTest1> t1;
  t1.appendTask ( new Task2b<DataTest1> );
  t1.appendTask ( new Task3<DataTest1> );
  DataTest1 d;
  d.modifiedbytask1 = d.modifiedbytask2 = d.modifiedbytask3 = false;
  bool result = t1.chainrun ( d );
  EXPECT_EQ ( result, true );
  EXPECT_EQ ( d.modifiedbytask1, true );
  EXPECT_EQ ( d.modifiedbytask2, true );
  EXPECT_EQ ( d.modifiedbytask3, false );
}

///Test that the functor works well appending tasks.
TEST ( TaskInterface, chainoftasks3 ) {
  Task1<DataTest1> t1;
  t1 ( new Task2b<DataTest1> )
  ( new Task3<DataTest1> );
  DataTest1 d;
  d.modifiedbytask1 = d.modifiedbytask2 = d.modifiedbytask3 = false;
  bool result = t1.chainrun ( d );
  EXPECT_EQ ( result, true );
  EXPECT_EQ ( d.modifiedbytask1, true );
  EXPECT_EQ ( d.modifiedbytask2, true );
  EXPECT_EQ ( d.modifiedbytask3, false );
}

///Test the introduction of a NULL in the chain of tasks. Task1 should point directly to task3 and hence both tasks get executed.

TEST ( TaskInterface, chainoftasks4 ) {
  Task1<DataTest1> t1;
  t1 ( NULL )
  ( new Task3<DataTest1> );
  DataTest1 d;
  d.modifiedbytask1 = d.modifiedbytask2 = d.modifiedbytask3 = false;
  bool result = t1.chainrun ( d );
  EXPECT_EQ ( result, false );
  EXPECT_EQ ( d.modifiedbytask1, true );
  EXPECT_EQ ( d.modifiedbytask2, false );
  EXPECT_EQ ( d.modifiedbytask3, true );
}

///Trivial data class, now contains unsigned and vector<unsigned>.
class DataTest2 {
 public:
  unsigned idx;
  std::vector<unsigned> v;
};

///Trivial task class, implements run to modify DataTest2.
template<class Data>
class Task4: public uu::TaskInterface<Data> {
 public:
  bool run ( Data& d ) {
    d.v.push_back ( d.idx );
    return false;
  };

};

///Test. As datatest not renewed, therefore we can accumulate information from a task on subsequent runs. vector<unsigned> should contain all k indices.
TEST ( TaskInterface, idx ) {
  Task4<DataTest2> t4;
  DataTest2 d;
  for ( unsigned k = 0; k < 5; k += 2 ) {
    d.idx = k;
    t4.run ( d );
  }
  ASSERT_EQ ( d.v.size(), 3 );
  EXPECT_EQ ( d.v[0], 0 );
  EXPECT_EQ ( d.v[1], 2 );
  EXPECT_EQ ( d.v[2], 4 );
}

/**
 * \brief This is a test to show how an imaginary postedit class could work.
 * The class would contain a pointer to the whole hifst system
 * A single run of PostEditTask may kickoff several times the decoder.
 */

template<class Data>
class PostEditTask: public uu::TaskInterface<Data> {
 private:
  uu::TaskInterface<Data> *fullsystem_;
 public:
  PostEditTask ( uu::TaskInterface<Data> *fs ) : fullsystem_ ( fs ) {};
  bool run ( Data& d ) {
    //Run first time on sentence x
    fullsystem_->chainrun ( d );
    // Do stuff...
    //Run second time on sentence x. d now contains certain modifications that could change the behaviour and output of the fullsystem.
    fullsystem_->chainrun ( d );
    return false;
  };
  inline ~PostEditTask() {
    if ( fullsystem_ ) delete fullsystem_;
  };

};

///Test. Task Class encapsulating another task(s). Executes twice for each index k -- vector<unsigned> contains repeated k values.
TEST ( TaskInterface, idx_chainrun ) {
  PostEditTask<DataTest2> t5 ( new Task4<DataTest2>() );
  DataTest2 d;
  for ( unsigned k = 0; k < 3; k += 2 ) {
    d.idx = k;
    t5.run ( d );
  }
  ASSERT_EQ ( d.v.size(), 4 );
  EXPECT_EQ ( d.v[0], 0 );
  EXPECT_EQ ( d.v[1], 0 );
  EXPECT_EQ ( d.v[2], 2 );
  EXPECT_EQ ( d.v[3], 2 );
}

};

#ifndef GMAINTEST

/**
 * \brief main function.
 * If compiled individualy, will kickoff any tests in this file.
 */

int main ( int argc, char **argv ) {
  ::testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
};

#endif
