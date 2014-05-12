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

/** \file include/taskinterface.hpp
 * \brief Interfaces with basic methods for iteration
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef TASKINTERFACE_HPP
#define TASKINTERFACE_HPP

namespace ucam {
namespace util {

/**
 * \brief Templated (hybrid) Interface for Task classes
 * \remark All task classes should inherit from TaskInterface and implement its virtual methods.
 * This interface is templated over the data, which is expected to be an arbitrary struct with several public variables
 * handled by this or other task classes.
 * TaskInterface allows to cascade tasks working over the same data instance, would easily also allow a mechanism similar
 * to chain of responsibility.
 */

template <class Data>
class TaskInterface {
 private:
  ///Next task class
  TaskInterface *next_;

 public:
  ///Constructor
  TaskInterface() : next_ ( NULL ) { };

  virtual bool run ( Data& d ) = 0;
  virtual ~TaskInterface() {
    if ( next_ ) delete next_;
  };

  /**
   * \brief Implements chain of responsability.
   * Calls run method and, if there is another task, call its run method too.
   * \param d Pass through
   * \return bool: true if the chain has been interrupted, false if normal execution.
   */

  inline bool chainrun   ( Data& d ) {
    bool r = run ( d );
    if ( !r  && next_ ) {
      return next_->chainrun ( d );
    };
    return r;
  };
  inline bool operator() ( Data& d ) {
    bool r = run ( d );
    if ( !r  && next_ ) {
      return next_->chainrun ( d );
    };
    return r;
  };

  /**
   * \brief Appends a task class.
   * If there is no task, append here, otherwise delegate in next task.
   * \param t Task to append.
   */
  inline TaskInterface& appendTask ( TaskInterface *t ) {
    if ( t == NULL ) return *this;
    if ( !next_ ) {
      next_ = t;
      return *next_;
    } else return ( *next_ ) ( t );
  };
  inline TaskInterface& operator() ( TaskInterface *t ) {
    if ( t == NULL ) return *this;
    if ( !next_ ) {
      next_ = t;
      return *next_;
    } else return ( *next_ ) ( t );
  };

  /**
   * \brief Return appended task.
   * \return Pointer to Task class stored in next_.
   */
  inline TaskInterface *getTask() {
    return next_;
  };
  inline TaskInterface *next() {
    return next_;
  };

};

/**
 * \brief Simple functor that accepts an interface and pointer to the data object in which it will have to run
 * The actual task running is delayed to the call of the (). This is useful e.g. for task dispatching in
 * the threadpool pattern. This functor deletes data and task as soon as it is guaranteed to have been completely executed.
 */

template<class Data>
class TaskFunctor {
 private:
  Data *d_;
  TaskInterface<Data> * task_;
 public:
  TaskFunctor()
    : d_ (NULL)
    , task_ (NULL) {
  };
  TaskFunctor ( TaskInterface<Data> *ti, Data *d )
    : d_ ( d )
    , task_ ( ti ) {
  };

  TaskFunctor (TaskFunctor<Data> const& tf)
    : d_ ( tf.d_)
    , task_ (tf.task_) {
  };

  void operator() () {
    if ( USER_CHECK ( task_ != NULL && d_ != NULL
                      , "TaskFunctor not properly initialized with a task and a data objects" ) ) {
      task_->chainrun ( *d_ );
      delete task_;
      delete d_;
      task_ = NULL;
      d_ = NULL;
    }
    return;
  };

};

}
} // end namespaces

#endif
