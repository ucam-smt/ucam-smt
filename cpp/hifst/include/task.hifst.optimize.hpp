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

#ifndef TASK_HIFST_OPTIMIZE_HPP
#define TASK_HIFST_OPTIMIZE_HPP

/**
 * \file
 * \brief Contains Function objects that optimize a machine
 */

namespace ucam {
namespace hifst {

template<class Arc = fst::LexStdArc>
class OptimizeMachine {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:
  bool alignmode_;

 public:
  ////Constructor
  OptimizeMachine ( bool align = false ) : alignmode_ (align)  {};
  inline void setAlignMode (bool align) {
    alignmode_ = align;
  };

  // If numstates is = to threshold, then the machine fully optimized
  // Impose any external condition in check => if it doesn'nt meet condition, then simply rmepsilons
  inline void operator() ( fst::VectorFst<Arc> *fst ,
                           uint nstatesthreshold = std::numeric_limits<uint>::max() ,
                           bool check = true ) const {
    if (fst->NumStates() > nstatesthreshold || ! check ) {
      LINFO ("Only rm epsilons...");
      fst::RmEpsilon<Arc> (fst);
      return;
    }
    LINFO ("Full optimization");
    this->optimize (fst);
  }

 private:

  inline void optimize ( fst::VectorFst<Arc> *fst ) const {
    if ( !alignmode_ ) {
      LINFO ("FSA");
      fst::Determinize (fst::RmEpsilonFst<Arc> (*fst), fst);
      fst::Minimize (fst);
    } else {
      LINFO ("FST");
      EncodeDeterminizeMinimizeDecode (fst::RmEpsilonFst<Arc> (*fst) , fst ) ;
    }
  };

  ZDISALLOW_COPY_AND_ASSIGN ( OptimizeMachine );
};

template<class Arc = fst::LexStdArc>
class OptimizeMachineNoDetMin {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:

 public:
  ////Constructor
  OptimizeMachineNoDetMin ( )  {};

  inline void operator() ( fst::VectorFst<Arc> *fst ) const {
    fst::RmEpsilon (fst);
  };

  inline void operator() ( fst::VectorFst<Arc> *fst ,
                           uint numstatesthreshold ) const {
    fst::RmEpsilon (fst);
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( OptimizeMachineNoDetMin );
};

}
} // end namespaces

#endif
