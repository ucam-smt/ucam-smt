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
string vmapfile;
bool printweight = false;
bool sparseformat = false;
bool nohyps = false;
bool liblinrankformat = false;
bool dotProduct = false;
unsigned myPrecision = 6; // default


using fst::Hyp;

// \brief Load intersection space which will be applied prior to printing
// If it is an fst, then load it and we are done.
// Otherwise, assume it is a list of integer-mapped hypotheses
// and build  an unweighted FSA.
template<class ArcT>
VectorFst<ArcT> *createIntersectionSpace(std::string const &filename) {
  if (filename == "") return NULL;
  if (fst::DetectFstFile(filename))
    return fst::VectorFstRead<ArcT>(filename);

  ucam::util::iszfstream f (filename);
  VectorFst<ArcT> *i = new VectorFst<ArcT>;
  i->AddState();
  i->SetStart(0);
  std::string sentence;

  while (getline(f,sentence)) {
    std::vector<std::string> wrds;
    boost::split(wrds, sentence, boost::is_any_of(" "));
    unsigned p = 0; // starts at state 0;
    for (unsigned k = 0; k < wrds.size(); ++k) {
      unsigned n =i->AddState();
      unsigned label = ucam::util::toNumber<unsigned>(wrds[k]);
      i->AddArc(p,ArcT(label, label, ArcT::Weight::One(), n));
      p=n;
    }
    i->SetFinal(p, ArcT::Weight::One());
  }
  f.close();
  *i = DeterminizeFst<ArcT>(*i);
  Minimize(i);
  return i;
}


/**
 * \brief Same as Hyp but the printing will convert integer ids to words.
 */
template<class Arc>
struct HypW: public Hyp<Arc> {

  HypW (std::basic_string<unsigned> const& h
        , std::basic_string<unsigned> const& oh
        , typename Arc::Weight const& c)
      : Hyp<Arc> (h, oh, c) {
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
void printWeight (typename Arc::Weight const& weight
                  , std::ostream& os
                  , unsigned precision = myPrecision
                  ) {
  os << std::setprecision(precision) << weight;
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
void printWeight<TupleArc32> (const TupleW32& weight
                              , std::ostream& os
                              , unsigned precision
                              ) {
  std::map<int,float> costs;
  std::string separator (",");

  for (fst::SparseTupleWeightIterator<fst::TropicalWeight, int> it (weight);
       !it.Done(); it.Next() ) {
    costs[it.Value().first] += it.Value().second.Value();
  }

  if (liblinrankformat) {
    for (std::map<int,float>::const_iterator itx=costs.begin();
	 itx != costs.end(); ++itx) {
      os << " " << itx->first << ":" << itx->second;
    }
    return;
  }

  if (sparseformat) {
    os << "0" << separator << costs.size();
    for (std::map<int,float>::const_iterator itx=costs.begin()
             ; itx != costs.end()
             ; ++itx) {
      os << separator << itx->first << separator << std::setprecision(precision) << itx->second;
    }
    return;
  } 
  if (dotProduct) {
    float w =0;
    std::vector<float> const &fws = TupleW32::Params();
    for (std::map<int,float>::const_iterator itx=costs.begin()
             ; itx != costs.end()
             ; ++itx) {
      if (itx->first < 1) continue;

      float fw = fws[itx->first - 1];
      w = w + fw * itx->second;
    }
    os << std::setprecision(precision) << w;
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
}

ucam::fsttools::SentenceIdx
RemoveUnprintable(const ucam::fsttools::SentenceIdx& h) {
  ucam::fsttools::SentenceIdx x;
  for (int k=0; k<h.size(); k++) {
    if (h[k] == OOV) continue;
    if (h[k] == DR) continue;
    if (h[k] == EPSILON) continue;
    if (h[k] == SEP) continue;
    if (h[k] == 1) continue;
    if (h[k] == 2) continue;
    x.push_back(h[k]);
  }
  return x;
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
    printWeight<Arc> (obj.cost, os, myPrecision);
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
    printWeight<Arc> (obj.cost, os, myPrecision);
  };
  return os;
}

template <class Arc, class HypT>
int run ( ucam::util::RegistryPO const& rg) {
  using namespace ucam::util;
  using namespace HifstConstants;
  PatternAddress<unsigned> input (rg.get<std::string>
                                  (kInput.c_str() ) );
  PatternAddress<unsigned> output (rg.get<std::string>
                                   (kOutput.c_str() ) );
  PatternAddress<unsigned> intersectionLattice (rg.get<std::string>
                                   (kIntersectionWithHypothesesLoad.c_str() ) );
  unsigned n = rg.get<unsigned> (kNbest.c_str() );
  boost::scoped_ptr<oszfstream> out;
  bool unique =  rg.exists (kUnique.c_str() );
  bool printOutputLabels = rg.exists(kPrintOutputLabels.c_str());
  bool printInputOutputLabels = rg.exists(kPrintInputOutputLabels.c_str());

  if (printInputOutputLabels)
    FORCELINFO("Printing input and output labels...");
  std::string old;
  std::string refFiles;
  bool intRefs = false;
  bool dobleu = false;
  bool sentbleu = false;

  if (rg.exists(HifstConstants::kWordRefs)) {
    refFiles = rg.getString(HifstConstants::kWordRefs);
    dobleu = true;
  }
  if (rg.exists(HifstConstants::kIntRefs)) {
    refFiles = rg.getString(HifstConstants::kIntRefs);
    intRefs = true;
    dobleu = true;
  }
  if (rg.exists (HifstConstants::kSentBleu) ) {
    if (!dobleu) {
      LERROR("Must provide references to compute sentence level bleu");
      exit(EXIT_FAILURE);
    }
    sentbleu = true;
  }

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
    dotProduct = true;
  }

  if (rg.exists (HifstConstants::kSuppress) ) {
    nohyps = true;
  }

  // n.b. this should be last, to override any other settings
  if (rg.exists (HifstConstants::kLibLinRankFormat) ) {
    liblinrankformat = true;
    if (!dobleu) {
      LERROR("Must provide references to compute features for liblinear rankings");
      exit(EXIT_FAILURE);
    }
    sentbleu = true;
  }

  std::string extTok(rg.exists(HifstConstants::kExternalTokenizer) ? 
		     rg.getString(HifstConstants::kExternalTokenizer) : "");
  ucam::fsttools::BleuStats bStats;
  ucam::fsttools::BleuScorer *bleuScorer;
  if (dobleu)
    bleuScorer = new ucam::fsttools::BleuScorer(refFiles, extTok, 1, intRefs, vmapfile);

  int nlines=0;
  for ( IntRangePtr ir (IntRangeFactory ( rg,
                                    kRangeOne ) );
        !ir->done();
        ir->next() ) {
    nlines++;
    boost::scoped_ptr<fst::VectorFst<Arc> > ifst (fst::VectorFstRead<Arc> (input (
          ir->get() ) ) );
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
    else if (!printInputOutputLabels) // what a mess
      fst::Project(&*ifst, PROJECT_INPUT);

    fst::VectorFst<Arc> nfst;
    // find 1-best and compute bleu stats
    if (dobleu) {
      ShortestPath (*ifst, &nfst, 1, unique);
      std::vector<HypT> hyps1;
      fst::printStrings<Arc> (nfst, &hyps1);
      ucam::fsttools::SentenceIdx h(hyps1[0].hyp.begin(), hyps1[0].hyp.end());
      // bleuscorer indexes references from 0; ir counts from 1
      if (h.size() > 0)
	bStats = bStats + bleuScorer->SentenceBleuStats(ir->get()-1, RemoveUnprintable(h));
    }

    boost::scoped_ptr< VectorFst<Arc> > intersection
        (createIntersectionSpace<Arc>( intersectionLattice( ir->get() ) ));


    if (intersection.get()) {
      VectorFst<Arc> cmps;
      *ifst = ComposeFst<Arc>(*intersection, *ifst);
    }

    if (!ifst->NumStates() ) {
      *out << "[EMPTY]" << std::endl;
      continue;
    }

    // Otherwise determinization runs (both determinizefst or
    // inside shortestpath) doesn't produce the expected result:
    // epsilons are being treated as symbols
    if (unique) {
      fst::RmEpsilon<Arc>(&*ifst);
    }
    // find nbest, compute stats, print
    ShortestPath (*ifst, &nfst, n, unique );

    std::vector<HypT> hyps;
    fst::printStrings<Arc> (nfst, &hyps);
    for (unsigned k = 0; k < hyps.size(); ++k) {
      ucam::fsttools::SentenceIdx h(hyps[k].hyp.begin(), hyps[k].hyp.end());	
      ucam::fsttools::BleuStats sbStats;
      double sbleu;
      if (sentbleu) {
	// bleuscorer indexes references from 0; ir counts from 1
	sbStats = bleuScorer->SentenceBleuStats(ir->get()-1, RemoveUnprintable(h));
	sbleu = bleuScorer->ComputeSBleu(sbStats).m_bleu;
      }
      // output
      if (liblinrankformat) {
	*out->getStream() << sbleu << " qid:" << ir->get();
	printWeight<Arc>(hyps[k].cost, *out->getStream());
	*out->getStream() << std::endl;
      } 
      if (nohyps == false) {
        if (printInputOutputLabels) { // add the output labels.
          //          for (unsigned k = 0; k < hyps.size(); ++k) {
            for (unsigned j = 0; j < hyps[k].hyp.size(); ++j)
              if (hyps[k].hyp[j] != 0)
                *out->getStream() << hyps[k].hyp[j] << " ";
            *out->getStream() << "\t";
            for (unsigned j = 0; j < hyps[k].ohyp.size(); ++j)
              if (hyps[k].ohyp[j] != 0)
                *out->getStream() << hyps[k].ohyp[j] << " ";
            if (printweight)
              *out->getStream() << "\t" << std::setprecision(myPrecision) << hyps[k].cost;
            //    *out->getStream() << std::endl;
            //          }
        } else {
          *out->getStream() << hyps[k];
        }
	if (sentbleu) 
	  *out->getStream() << "\t" << sbStats << "\t" << sbleu;
	*out->getStream() << std::endl;
      }
    }
  }
  if (dobleu)
    FORCELINFO("BLEU STATS:" << bStats << "; BLEU: " << bleuScorer->ComputeBleu(bStats));
  FORCELINFO("Processed " << nlines << " files");
};

/*
 * \brief Main function.
 * \param       argc Number of command-line program options.
 * \param       argv Actual program options.
 * \remarks
 */
int  main ( int argc, const char* argv[] ) {
  using namespace HifstConstants;
  using namespace ucam::util;
  initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "=====================" ) );
  if (rg.exists (kWeight) ) {
    printweight = true;
    myPrecision = rg.get<unsigned>(kWeightPrecision.c_str());
    LINFO("Setting float precision=" << myPrecision);
  }
  if (rg.exists (kSparseFormat) ) {
    sparseformat = true;
  }
  if (rg.exists (kSparseDotProduct) ) {
    if (sparseformat == true) {
      LERROR("Sparse format and dot product are not available at the same time.");
      exit(EXIT_FAILURE);
    }
    dotProduct = true;
  }

  // check that tuplearc weights are set for the tuplearc semiring
  if (rg.get<std::string> (kHifstSemiring.c_str() ) ==
      kHifstSemiringTupleArc) {
    const std::string& tuplearcWeights = rg.exists (
                                           kTupleArcWeights)
                                         ? rg.get<std::string> (kTupleArcWeights.c_str() ) : "";
    if (tuplearcWeights.empty() ) {
      LERROR ("The tuplearc.weights option needs to be specified "
              "for the tropical sparse tuple weight semiring "
              "(--semiring=tuplearc)");
      exit (EXIT_FAILURE);
    }
    TupleW32::Params() = ParseParamString<float> (tuplearcWeights);
  }
  std::string const& semiring = rg.get<std::string>
                                (kHifstSemiring);
  if (!vmap.size() && rg.get<std::string> (kLabelMap) != "" ) {
    FORCELINFO ("Loading symbol map file...");
    vmapfile = rg.get<std::string> (HifstConstants::kLabelMap);
    iszfstream f (rg.get<std::string> (kLabelMap) );
    unsigned id;
    std::string word;
    while (f >> word >> id) {
      vmap[id] = word;
    }
    FORCELINFO ("Loaded " << vmap.size() << " symbols");
    if (semiring == kHifstSemiringStdArc) {
      run<fst::StdArc, HypW<fst::StdArc> > (rg);
    } else if (semiring == kHifstSemiringLexStdArc) {
      run<fst::LexStdArc, HypW<fst::LexStdArc> > (rg);
    } else if (semiring == kHifstSemiringTupleArc) {
      run<TupleArc32, HypW<TupleArc32> > (rg);
    } else {
      LERROR ("Sorry, semiring option not correctly defined");
    }
    FORCELINFO ( argv[0] << " finished!" );
    exit (EXIT_SUCCESS);
  }
  if (semiring == kHifstSemiringStdArc) {
    run<fst::StdArc, Hyp<fst::StdArc> > (rg);
  } else if (semiring == kHifstSemiringLexStdArc) {
    run<fst::LexStdArc, Hyp<fst::LexStdArc> > (rg);
  } else if (semiring == kHifstSemiringTupleArc) {
    run<TupleArc32, Hyp<TupleArc32> > (rg);
  } else {
    LERROR ("Sorry, semiring option not correctly defined");
  }
  FORCELINFO ( argv[0] << " finished!" );
}
