#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main.samplehyps.hpp>

using fst::Hyp;

template<class Arc>
struct HypW: public Hyp<Arc> {

  HypW (std::basic_string<unsigned> const& h, typename Arc::Weight const& c)
    : Hyp<Arc> (h, c) {
  }
  HypW (HypW<Arc> const& h)
    : Hyp<Arc> (h) {
  }
};

template<class Arc>
std::ostream& operator<< (std::ostream& os, const Hyp<Arc>& obj) {
  for (unsigned k = 0; k < obj.hyp.size(); ++k) {
    if (obj.hyp[k] == OOV) continue;
    if (obj.hyp[k] == DR) continue;
    if (obj.hyp[k] == EPSILON) continue;
    if (obj.hyp[k] == SEP) continue;
    os << obj.hyp[k] << " ";
  }
  os << "\t";
  os << obj.cost;
  return os;
};


class Sample {
public:
  Sample(const unsigned j1, const unsigned j2, const double scoreDiff) :
    j1(j1), j2(j2), scoreDiff(scoreDiff) {}

  unsigned int j1;
  unsigned int j2;
  double scoreDiff;
};

bool SampleSortPredicate(const Sample& s1, const Sample& s2) {
	return s1.scoreDiff > s2.scoreDiff;
}

template <class HypT>
ucam::fsttools::Bleu LBleuScorer(ucam::fsttools::BleuScorer& bleuScorer, unsigned const& sid, HypT const& hyp) {
  ucam::fsttools::SentenceIdx h;
  unsigned offset = (hyp.size() < 2 ? 0 : 1);
  for (unsigned z=offset; z< hyp.size()-offset; z++)
    h.push_back( hyp[z] );
  return bleuScorer.ComputeSBleu(bleuScorer.SentenceBleuStats(sid, h));
}

template <class Value, class Weight>
struct LabeledFeature {
  Value value;
  Weight fea;
};

template <class Weight, class HypT>
vector< LabeledFeature<float, Weight> > 
ProSBLEUSample(ucam::fsttools::BleuScorer& bleuScorer, 
	       std::vector<HypT> const& hyps, unsigned const& sid, 
	       unsigned const& n, unsigned const &ns, double const& alpha, bool negatives=false, bool negate=true ) {

  std::set< std::pair<unsigned, unsigned> > indexpairs;
  vector< Sample > samples;
  for (unsigned s=0; s<n; s++) {
    LINFO("s="<<s);
    unsigned j1 = rand() % hyps.size();
    unsigned j2 = rand() % hyps.size();
    LINFO("1 [" << j1 <<"] " <<hyps[j1]);
    LINFO("2: ["<<j2<<"] "<<hyps[j2]);
    if (indexpairs.find( std::make_pair(j1, j2)) != indexpairs.end() ) {
      LINFO("--skipping - already done");
      continue;
    }
    indexpairs.insert( std::make_pair(j1, j2) );
    ucam::fsttools::Bleu bs1 = LBleuScorer(bleuScorer, sid, hyps[j1].hyp);
    ucam::fsttools::Bleu bs2 = LBleuScorer(bleuScorer, sid, hyps[j2].hyp);
    LINFO("SBLUE1= "<<bs1.m_bleu<<" ; SBLEU2="<<bs2.m_bleu << " ; DIFF=" << fabs(bs1.m_bleu - bs2.m_bleu));
    if ( bs1.m_bleu - bs2.m_bleu > alpha ) 
      samples.push_back(Sample(j1, j2, bs1.m_bleu - bs2.m_bleu));
    if ( bs2.m_bleu - bs1.m_bleu > alpha ) {
      samples.push_back(Sample(j2, j1, bs2.m_bleu - bs1.m_bleu));
    }
  }
  LINFO("Positive samples found: " << samples.size());
  sort(samples.begin(), samples.end(), SampleSortPredicate);
  std::vector< LabeledFeature< float, Weight> > ss;
  int np=0;
  for (unsigned s=0; s<ns && s < samples.size(); s++) {
    unsigned j1 = samples[s].j1;
    unsigned j2 = samples[s].j2;
    if (negate) {
      unsigned x = j1;
      j1 = j2;
      j2 = x;
    }
    LabeledFeature<float, Weight> lf;
    lf.value = samples[s].scoreDiff;
    lf.fea = Divide(hyps[j1].cost, hyps[j2].cost);
    ss.push_back(lf);
    LINFO("Sample " << s << " score diff " << samples[s].scoreDiff);
    np++;
    if (!negatives) 
      continue;
    lf.value = -samples[s].scoreDiff;
    lf.fea = Divide(hyps[j2].cost, hyps[j1].cost);
    ss.push_back(lf);
  }
  LINFO("Positive samples found: " << np << " of " << n);
  return ss;
};

template <class Arc, class HypT>
int SampleWFSAs( ucam::util::RegistryPO const& rg) {
  using ucam::util::oszfstream;
  using ucam::util::PatternAddress;
  PatternAddress<unsigned> input(rg.get<std::string>(HifstConstants::kInput.c_str()));
  PatternAddress<unsigned> output(rg.get<std::string>(HifstConstants::kOutput.c_str()));
  unsigned n = rg.get<unsigned>(HifstConstants::kNbest.c_str());
  unsigned ns = rg.get<unsigned>(HifstConstants::kNSamples.c_str()); 
  float alpha = rg.get<float>(HifstConstants::kAlpha.c_str()); 
  bool negatives = rg.exists(HifstConstants::kNegativeExamples.c_str()); 
  bool binarytarget = rg.exists(HifstConstants::kBinaryTarget.c_str());
  bool negate = !rg.exists(HifstConstants::kDontNegate.c_str());
  std::string extTok = rg.getString(HifstConstants::kExternalTokenizer.c_str());
  std::string wMap   = rg.getString(HifstConstants::kWordMap.c_str());
  //  std::string wMap = "";
  //
  bool printOutputLabels = rg.exists(HifstConstants::kPrintOutputLabels.c_str());
  std::string refFiles;
  bool intRefs;
  if (rg.exists(HifstConstants::kWordRefs)) {
    refFiles = rg.getString(HifstConstants::kWordRefs);    
    intRefs = false;
  }
  if (rg.exists(HifstConstants::kIntRefs)) {
    refFiles = rg.getString(HifstConstants::kIntRefs);
    intRefs = true;
  } 
  ucam::fsttools::BleuScorer bleuScorer(refFiles, extTok, n, intRefs, wMap);
  ucam::fsttools::TuneSet< Arc > tuneSet(rg);
  ucam::fsttools::Bleu ibs = tuneSet.ComputeBleu(bleuScorer);
  FORCELINFO("Set level Bleu: " << ibs);
  unsigned seed = time(NULL);
  if (rg.exists(HifstConstants::kRandomSeed.c_str()))
    seed = rg.get<unsigned>(HifstConstants::kRandomSeed.c_str());
  FORCELINFO("random seed: " << seed);
  srand(seed);
  boost::scoped_ptr<oszfstream> out;
  std::string old;
  for (unsigned i=0; i<tuneSet.cachedLats.size(); i++) {
    fst::VectorFst<Arc> ifst(*tuneSet.cachedLats[i]);
    fst::VectorFst<Arc> nfst;
    if (old != output (i) ) {
      out.reset(new oszfstream (output(i)));
      old = output(i);
    }
    if (!ifst.NumStates() ) {
      FORCELINFO("EMPTY: " << i);
      continue;
    }
    // Projecting allows unique to work for all cases.
    fst::Project(&ifst, (printOutputLabels?PROJECT_OUTPUT:PROJECT_INPUT));
    ShortestPath (ifst, &nfst, n, true );
    std::vector<HypT> hyps;
    fst::printStrings<Arc> (nfst, &hyps);
    std::vector< LabeledFeature< float, typename Arc::Weight> > fea = 
      ProSBLEUSample<typename Arc::Weight, HypT>(bleuScorer, hyps, i, n, ns, alpha, negatives, negate);
    for (unsigned s=0; s<fea.size(); s++) {
      *out << (binarytarget ? (fea[s].value > 0.0 ? 1 : 0) : fea[s].value);
      *out << " " << fea[s].fea << std::endl;
    }
  }
  FORCELINFO("Done Sample WFSAs");
};



int  main ( int argc, const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "====================="));
  std::string const& semiring = rg.get<std::string> (HifstConstants::kHifstSemiring);
  if (semiring == HifstConstants::kHifstSemiringStdArc) {
    SampleWFSAs<fst::StdArc, Hyp<fst::StdArc> > (rg);
    FORCELINFO("semiring StdArc");
  } else if (semiring == HifstConstants::kHifstSemiringLexStdArc) {
    FORCELINFO("semiring LexStdArc");
    SampleWFSAs<fst::LexStdArc, Hyp<fst::LexStdArc> > (rg);
  } else if (semiring == HifstConstants::kHifstSemiringTupleArc) {
    FORCELINFO("semiring TupleArc32");
    const std::string& tuplearcWeights = 
      (rg.exists(HifstConstants::kTupleArcWeights) 
       ? rg.get<std::string> (HifstConstants::kTupleArcWeights.c_str()) : "");
    if (tuplearcWeights.empty() ) {
      LERROR ("The tuplearc.weights option needs to be specified "
              "for the tropical sparse tuple weight semiring "
              "(--semiring=tuplearc)");
      exit (EXIT_FAILURE);
    }
    TupleW32::Params() = ucam::util::ParseParamString<float> (tuplearcWeights);
    SampleWFSAs<TupleArc32, Hyp<TupleArc32> > (rg);
  } else {
    LERROR ("Sorry, semiring option not correctly defined");
  }
  FORCELINFO ( argv[0] << " finished!" );
}

