#ifndef TASK_HIFST_MAKEWEIGHTS_HPP
#define TASK_HIFST_MAKEWEIGHTS_HPP

namespace ucam {
namespace hifst {

// @todo: this codes needs to be merged with other makeweight functors.
// General template class. This returns error.
template<class Arc>
struct MakeWeightHifst {
  typedef typename Arc::Weight Weight;
  explicit MakeWeightHifst(const ucam::util::RegistryPO& rg) {
    LERROR("MakeWeightHifst not implemented for this semiring");
    exit(EXIT_FAILURE);
  }
  inline Weight operator () (float const weight )  const {
    return Weight ( weight );
  };
  inline Weight operator () (Weight const & weight) const  {
    return weight ;
  };
  inline Weight operator () (float const weight, unsigned)  const {
    return Weight(weight);
  };

  inline void update() {};

};

// Template specialization for Lexicographic StdArc,StdArc.
// This is essentially the logic in lexicographic-tropical-tropical-funcs.h
template<>
struct MakeWeightHifst<fst::LexStdArc> {
  typedef fst::LexStdArc Arc;
  typedef Arc::Weight Weight;
  explicit MakeWeightHifst(const ucam::util::RegistryPO& rg) {
  }

  inline Weight operator () (float const weight )  const {
    return Weight ( weight, weight);
  };
  inline Weight operator () (Weight const & weight) const  {
    return Weight( weight.Value2(), weight.Value2() );
  };
  inline Weight operator () (float const weight, unsigned)  const {
    return Weight(weight, weight);
  };
  inline void update() {};
};

// Template specialization for TupleArc
template<>
struct MakeWeightHifst<TupleArc32> {
  typedef TupleArc32 Arc;
  typedef Arc::Weight Weight;

  bool addRuleFeature_;
  // sparse feature index.
  // +1 because sparse tuple semiring starts at 1, not 0.
  unsigned k_;
  // Assumptions: the tropical sparse tuple semiring
  // will carry first the lm weights, then a single grammar weight (dot product)
  // in addition, rule indices are passed as features (negative indices, get
  // ignored by the dot product).
  MakeWeightHifst(const ucam::util::RegistryPO& rg)
      : addRuleFeature_(!rg.getBool(HifstConstants::kHifstDisableRuleFeatures))
      , k_(rg.getVectorString(HifstConstants::kLmFeatureweights).size() + 1)
  {}

  inline Weight operator () (float const weight )  const {
    Weight result;
    result.Push ( k_, weight);
    return result;
  };

  inline Weight operator () (float const weight, unsigned spi )  const {
    Weight result;
    result.Push ( k_ , weight);
    if (addRuleFeature_)
      result.Push( -spi - 2, 1);
    return result;
  };

  inline Weight operator () (Weight const &weight) const  {
    return weight;
  };

  inline void update() {};
};

// General template class. This returns error.
template<class Arc>
struct MakeWeightHifstLocalLm {
  typedef typename Arc::Weight Weight;
  explicit MakeWeightHifstLocalLm() {}
  explicit MakeWeightHifstLocalLm(const ucam::util::RegistryPO& rg) {
    LERROR("MakeWeightHifstLocalLm not implemented for this semiring");
    exit(EXIT_FAILURE);
  }
  inline Weight operator () (float const weight )  const {
    return Weight ( weight );
  };
  inline Weight operator () (Weight const & weight) const  {
    return weight ;
  };
  inline Weight operator () (float const weight, unsigned)  const {
    return Weight(weight);
  };
  inline void update() {};
};

// Template specialization for Lexicographic StdArc,StdArc.
// This is essentially the logic in lexicographic-tropical-tropical-funcs.h
template<>
struct MakeWeightHifstLocalLm<fst::LexStdArc> {
  typedef fst::LexStdArc Arc;
  typedef Arc::Weight Weight;
  typedef fst::StdArc::Weight StdWeight;
  explicit MakeWeightHifstLocalLm() {}
  explicit MakeWeightHifstLocalLm(const ucam::util::RegistryPO& rg) {
  }

  // Language Model uses this one:
  inline Weight operator () (float const weight )  const {
    return Weight ( weight, StdWeight::One());
  };
  inline Weight operator () (float const weight, unsigned)  const {
    return Weight(weight, StdWeight::One() );
  };

  // copies back the second cost into the first one.
  inline Weight operator () (Weight const & weight) const  {
    return Weight( weight.Value2(), weight.Value2() );
  };
  inline void update() {};
};

// singleton-like function to hack in the Mapper state the actual
// position of the local lm in tuple arc 32.
// Assumptions: the tropical sparse tuple semiring
// will carry first the lm weights, then a single grammar weight (dot product)
// the next feature weight is the local language model.
// offset +1
inline int getLocalLmIndex(ucam::util::RegistryPO const *rg = NULL) {
  static int k = -1;
  if (rg != NULL)
    k = rg->getVectorString(HifstConstants::kLmFeatureweights).size() + 1 + 1;
  return k;
};

// Map does not copy the mapper functor, so an internal variable will not get
// initialized properly. Hence, using getLocalLmIndex to initialize.
// Template specialization for TupleArc
template<>
struct MakeWeightHifstLocalLm<TupleArc32> {
  typedef TupleArc32 Arc;
  typedef Arc::Weight Weight;

  unsigned k_;
  explicit MakeWeightHifstLocalLm()
      : k_(getLocalLmIndex())
  {
    if (!k_) {
      LERROR("MakeWeightHifstLocalLm -- Index has to be bigger than 0 (constr)");
      exit(EXIT_FAILURE);
    }
  }
  MakeWeightHifstLocalLm(const ucam::util::RegistryPO& rg)
      : k_(getLocalLmIndex(&rg))
  {}

  // More than one language model will add scores to the same k_
  inline Weight operator () (float const weight )  const {
    if (!k_) {
      LERROR("MakeWeightHifstLocalLm -- Index has to be bigger than 0 (op)");
      exit(EXIT_FAILURE);
    }
    Weight result;
    result.Push ( k_, weight);
    return result;
  };
  // deletes lm scores under tropical sparse tuple weights
  // not as effective as lexicographic semiring, but should do the job.
  inline Weight operator () (Weight const &weight) const  {
    if (!k_) {
      LERROR("MakeWeightHifstLocalLm -- Index has to be bigger than 0! (op 2)");
      exit(EXIT_FAILURE);
    }
    using namespace fst;
    Weight result;
    result.SetDefaultValue(weight.DefaultValue());
    for (SparseTupleWeightIterator<StdArc::Weight, int> it(weight); !it.Done(); it.Next()) {
      if (it.Value().first != k_)
        result.Push(it.Value());
    }
    return result;
  };

  // local language model always working on the default value(s)
  inline void update() {};
};

}} // end namespaces

#endif
