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

#ifndef TASK_OPTIMIZEFST_HPP
#define TASK_OPTIMIZEFST_HPP

/**
 * \file
 * \brief Implementation of a Fst writer taking the fst from data object
 * \date 10-12-2014
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/**
 * \brief Convenience class that inherits Taskinterface behaviour and
 * optimizes an fst.
 */
template <class Data, class Arc = fst::StdArc >
class OptimizeFstTask: public ucam::util::TaskInterface<Data> {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:
  ///key to access fst in the data object
  std::string fstkey_;

  bool stripHifstEpsilons_;
  fst::RelabelUtil<Arc> ru_;
 public:
  ///Constructor with RegistryPO object
  OptimizeFstTask ( const ucam::util::RegistryPO& rg
                    , const std::string& fstkey
		    , const std::string stripEps
                    )
      : fstkey_ (fstkey)
      , stripHifstEpsilons_(rg.getBool (stripEps) )
  {
    if (!stripHifstEpsilons_) return;
    ru_.addIPL (DR, EPSILON)
      .addIPL (OOV, EPSILON)
      .addIPL (SEP, EPSILON)
      .addOPL (DR, EPSILON)
      .addOPL (OOV, EPSILON)
      .addOPL (SEP, EPSILON)
      ;
  };

  inline static OptimizeFstTask * init ( const ucam::util::RegistryPO& rg
					 , const std::string& optimizefstkey
					 , const std::string& fstkey
					 , const std::string& stripepskey
					 ) {
    if ( rg.getBool ( optimizefstkey ) ) 
      return new OptimizeFstTask ( rg, fstkey, stripepskey );
    return NULL;
  };

  /**
   * \brief Optimizes fst. this involves rmepsilon, determinizing and minimizing.
   * The fst is accessed via data object using access key readfstkey_.
   * If parentheses exist, the procedure is skipped, as a PDT is not determinizable.
   * \param &d: data object
   * \returns false (does not break in any case the chain of tasks)
   */
  inline bool run ( Data& d ) {
    using namespace fst;
    VectorFst<Arc> *auxfst = d.getFst(fstkey_);
    if (auxfst == NULL) {
      LERROR("Lattice " << d.sidx << "not available under key " << fstkey_);
      exit(EXIT_FAILURE);
    }
    std::string parenskey = fstkey_ + ".parens";
    if ( d.fsts.find ( parenskey ) != d.fsts.end() ) {
      LWARN("Skipping optimization! PDTs are, in general, non-determinizable.");
      return false;
    }
    if (stripHifstEpsilons_) {
      FORCELINFO ("Remove hifst epsilons");
      ru_(auxfst);
    }
    FORCELINFO("Rm/Det/Min lattice " << d.sidx );
    Determinize(RmEpsilonFst<Arc>(*auxfst), auxfst);
    Minimize(auxfst);
    return false;
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( OptimizeFstTask );
};

}}  // end namespaces

#endif
