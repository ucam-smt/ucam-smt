
/**
 * \file
 * \brief Main file for printstrings tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main.printstrings.hpp>

typedef std::tr1::unordered_map<std::size_t, std::string> labelmap_t;
typedef labelmap_t::iterator labelmap_iterator_t;
labelmap_t vmap;
bool printweight = false;
bool sparseformat = false;
bool dotproduct = false;

using fst::Hyp;

/**
 * \brief Same as Hyp but the printing will convert integer ids to words.
 */
template<class Arc>
struct HypW: public Hyp<Arc> {

  HypW (std::basic_string<unsigned> const& h, typename Arc::Weight const& c)
    : Hyp<Arc> (h, c) {
  }
  HypW (HypW<Arc> const& h)
    : Hyp<Arc> (h) {
  }
};

/**
 * \brief Templated method that prints an arc weight. By default, reuses
 * the operator<< already defined for each weight.
 * \param weight The arc weight to be printed
 * \param os The output stream to print to
 */
template <class Arc>
void printWeight (typename Arc::Weight const& weight, std::ostream& os) {
  os << weight;
}

/**
 * \brief Template specialization of printWeight for a tropical sparse
 * tuple weight. Uses the global var sparseformat.
 * For the non sparse format, the comma separator is hard coded.
 * \param weight The arc weight to be printed
 * \param sparseformat Whether to print in sparse format or not.
 * \param os The output stream to print to
 */
template <>
void printWeight<TupleArc32> (const TupleW32& weight, std::ostream& os) {
  std::map<int,float> costs;
  std::string separator (",");

  for (fst::SparseTupleWeightIterator<fst::TropicalWeight, int> it (weight);
       !it.Done(); it.Next() ) {
    costs[it.Value().first] += it.Value().second.Value();
  }
  if (sparseformat) {

    os << "0" << separator << costs.size();
    for (std::map<int,float>::const_iterator itx=costs.begin()
             ; itx != costs.end()
             ; ++itx) {
      os << separator << itx->first << separator << itx->second;
    }
    return;
  } 
  if (dotproduct) {
    float w =0;
    std::vector<float> const &fws = TupleW32::Params();
    for (std::map<int,float>::const_iterator itx=costs.begin()
             ; itx != costs.end()
             ; ++itx) {
      if (itx->first < 1) continue;

      float fw = fws[itx->first - 1];
      w = w + fw * itx->second;
    }
    os << w;
    return;
  }
  std::size_t nonSparseSize = TupleW32::Params().size();
  std::size_t counter = 1;
  separator = "";
  for (std::map<int,float>::const_iterator itx=costs.begin()
           ; itx != costs.end()
           ; ++itx) {
    if (itx->first < 1 ) continue;
    std::size_t featureIndex = itx->first;
    for (std::size_t featureMissingIndex = counter;
         featureMissingIndex < featureIndex; ++featureMissingIndex) {
      os << separator << "0";
      separator = ","; // @todo should be possible to avoid resetting every time.
    }
    os << separator << itx->second;
    counter = itx->first + 1;
    separator = ",";
  }
  for (; counter <= nonSparseSize; ++counter) {
    os << separator << "0";
    separator = ",";
  }
  // dense format is buggy. We also want sparse format to sum contributions to the same index.
  // if (sparseformat) {
  //   os << weight;
  // } else {
  //   // size of the parameter vector
  //   std::size_t nonSparseSize = TupleW32::Params().size();
  //   std::size_t counter = 1;
  //   std::string separator ("");
  //   for (fst::SparseTupleWeightIterator<fst::TropicalWeight, int> it (weight);
  //        !it.Done(); it.Next() ) {
  //     std::size_t featureIndex = it.Value().first;
  //     for (std::size_t featureMissingIndex = counter;
  //          featureMissingIndex < featureIndex; ++featureMissingIndex) {
  //       os << separator << "0";
  //       separator = ",";
  //       counter++;
  //     }
  //     os << separator << it.Value().second;
  //     separator = ",";
  //     counter++;
  //   }
  //   for (; counter <= nonSparseSize; ++counter) {
  //     os << separator << "0";
  //     separator = ",";
  //   }
  // }
}

/**
 * \brief Operator<< overloading to print a hypothesis.
 */
template<class Arc>
std::ostream& operator<< (std::ostream& os, const Hyp<Arc>& obj) {
  for (unsigned k = 0; k < obj.hyp.size(); ++k) {
    if (obj.hyp[k] == OOV) continue;
    if (obj.hyp[k] == DR) continue;
    if (obj.hyp[k] == EPSILON) continue;
    if (obj.hyp[k] == SEP) continue;
    os << obj.hyp[k] << " ";
  }
  if (printweight) {
    os << "\t";
    printWeight<Arc> (obj.cost, os);
  };
  return os;
}

/**
 * \brief Operator<< overloading to print a hypothesis.
 * Integer are converted to words using the global variable wmap.
 */
template<class Arc>
std::ostream& operator<< (std::ostream& os, const HypW<Arc>& obj) {
  for (unsigned k = 0; k < obj.hyp.size(); ++k) {
    if (obj.hyp[k] == OOV) continue;
    if (obj.hyp[k] == DR) continue;
    if (obj.hyp[k] == EPSILON) continue;
    if (obj.hyp[k] == SEP) continue;
    labelmap_iterator_t itx = vmap.find (obj.hyp[k]);
    if (itx != vmap.end() )
      os << vmap[ obj.hyp[k] ] << " ";
    else {
      os << "[" << obj.hyp[k] << "] ";
      std::cerr << "\nWARNING: word map does not contain word " << obj.hyp[k] <<
                std::endl;
    }
  }
  if (printweight) {
    os << "\t";
    printWeight<Arc> (obj.cost, os);
  };
  return os;
}

template <class Arc, class HypT>
int run ( ucam::util::RegistryPO const& rg) {
  using ucam::util::oszfstream;
  using ucam::util::PatternAddress;
  PatternAddress<unsigned> input (rg.get<std::string>
                                  (HifstConstants::kInput.c_str() ) );
  PatternAddress<unsigned> output (rg.get<std::string>
                                   (HifstConstants::kOutput.c_str() ) );
  unsigned n = rg.get<unsigned> (HifstConstants::kNbest.c_str() );
  boost::scoped_ptr<oszfstream> out;
  bool unique =  rg.exists (HifstConstants::kUnique.c_str() );
  bool printOutputLabels = rg.exists(HifstConstants::kPrintOutputLabels.c_str());
  std::string old;
  for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg,
                                    HifstConstants::kRangeOne ) );
        !ir->done();
        ir->next() ) {
    FORCELINFO ("Processing file " << input ( ir->get() ) );
    boost::scoped_ptr<fst::VectorFst<Arc> > ifst (fst::VectorFstRead<Arc> (input (
          ir->get() ) ) );
    fst::VectorFst<Arc> nfst;
    Connect (&*ifst);
    if (old != output (ir->get() ) ) {
      out.reset (new oszfstream (output (ir->get() ) ) );
      old = output (ir->get() );
    }
    if (!ifst->NumStates() ) {
      *out << "[EMPTY]" << std::endl;
      continue;
    }
    // Projecting allows unique to work for all cases.
    if (printOutputLabels)
      fst::Project(&*ifst, PROJECT_OUTPUT);
    else
      fst::Project(&*ifst, PROJECT_INPUT);
    ShortestPath (*ifst, &nfst, n, unique );
    std::vector<HypT> hyps;
    fst::printStrings<Arc> (nfst, &hyps);
    for (unsigned k = 0; k < hyps.size(); ++k) {
      *out->getStream() << hyps[k] << std::endl;
    }
  }
};

/*
 * \brief Main function.
 * \param       argc Number of command-line program options.
 * \param       argv Actual program options.
 * \remarks
 */
int  main ( int argc, const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "=====================" ) );
  if (rg.exists (HifstConstants::kWeight) ) {
    printweight = true;
  }
  if (rg.exists (HifstConstants::kSparseFormat) ) {
    sparseformat = true;
  }
  if (rg.exists (HifstConstants::kSparseDotProduct) ) {
    if (sparseformat == true) {
      LERROR("Sparse format and dot product are not available at the same time.");
      exit(EXIT_FAILURE);
    }
    dotproduct = true;
  }
  // check that tuplearc weights are set for the tuplearc semiring
  if (rg.get<std::string> (HifstConstants::kHifstSemiring.c_str() ) ==
      HifstConstants::kHifstSemiringTupleArc) {
    const std::string& tuplearcWeights = rg.exists (
                                           HifstConstants::kTupleArcWeights)
                                         ? rg.get<std::string> (HifstConstants::kTupleArcWeights.c_str() ) : "";
    if (tuplearcWeights.empty() ) {
      LERROR ("The tuplearc.weights option needs to be specified "
              "for the tropical sparse tuple weight semiring "
              "(--semiring=tuplearc)");
      exit (EXIT_FAILURE);
    }
    TupleW32::Params() = ucam::util::ParseParamString<float> (tuplearcWeights);
  }
  std::string const& semiring = rg.get<std::string>
                                (HifstConstants::kHifstSemiring);
  if (!vmap.size() && rg.get<std::string> (HifstConstants::kLabelMap) != "" ) {
    FORCELINFO ("Loading symbol map file...");
    ucam::util::iszfstream f (rg.get<std::string> (HifstConstants::kLabelMap) );
    unsigned id;
    std::string word;
    while (f >> word >> id) {
      vmap[id] = word;
    }
    FORCELINFO ("Loaded " << vmap.size() << " symbols");
    if (semiring == HifstConstants::kHifstSemiringStdArc) {
      run<fst::StdArc, HypW<fst::StdArc> > (rg);
    } else if (semiring == HifstConstants::kHifstSemiringLexStdArc) {
      run<fst::LexStdArc, HypW<fst::LexStdArc> > (rg);
    } else if (semiring == HifstConstants::kHifstSemiringTupleArc) {
      run<TupleArc32, HypW<TupleArc32> > (rg);
    } else {
      LERROR ("Sorry, semiring option not correctly defined");
    }
    FORCELINFO ( argv[0] << " finished!" );
    exit (EXIT_SUCCESS);
  }
  if (semiring == HifstConstants::kHifstSemiringStdArc) {
    run<fst::StdArc, Hyp<fst::StdArc> > (rg);
  } else if (semiring == HifstConstants::kHifstSemiringLexStdArc) {
    run<fst::LexStdArc, Hyp<fst::LexStdArc> > (rg);
  } else if (semiring == HifstConstants::kHifstSemiringTupleArc) {
    run<TupleArc32, Hyp<TupleArc32> > (rg);
  } else {
    LERROR ("Sorry, semiring option not correctly defined");
  }
  FORCELINFO ( argv[0] << " finished!" );
}
