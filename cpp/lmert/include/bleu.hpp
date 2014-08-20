#ifndef BLEU_HPP
#define BLEU_HPP


class Bleu {
public:
  Bleu (const double bleu = -std::numeric_limits<double>::infinity(), const double brev = 0) : m_bleu (bleu), m_brev (brev) {}
  inline double GetError() const {
    return m_bleu;
  }
  
  double m_bleu;
  double m_brev;
};

std::ostream& operator<< (std::ostream& o, const Bleu& b) {
  o << b.m_bleu << " (" << b.m_brev << ")";
  return o;
}

bool operator> (Bleu& b1, Bleu& b2) {
  return b1.m_bleu > b2.m_bleu;
}

// BleuStats hash functions
#define MODULUS__ 1000000
class hashfvecint64 : public
  std::unary_function<std::vector<long long>, std::size_t> {
 public:
  std::size_t operator() (std::vector<long long> const& v) const {
    unsigned int res = 0;
    for (unsigned int i = 0; i < v.size(); ++i)  
      res += (std::size_t) pow ( (double) v[i], double ( ( (i) % 6) + 1) );
    return res % MODULUS__;
  }
};
class hasheqvecint64 {
 public:
  bool operator() (std::vector<long long> const& v1, std::vector<long long> const& v2) const {
    return v1 == v2;
  }
};

class BleuStats {
public:
  static const unsigned int MAX_BLEU_ORDER = 4;
  vector<int> tots_; // order-specific ngram totals
  vector<int> hits_; // order-specific ngram hits
  long refLength_; // effective reference length

  BleuStats (): refLength_(0) {
    tots_.resize(MAX_BLEU_ORDER, 0);
    hits_.resize(MAX_BLEU_ORDER, 0);
  }

  BleuStats (const std::vector<int> tots, const std::vector<int> hits, const long refLength): 
    tots_(tots), hits_(hits), refLength_(refLength){}
};

std::ostream& operator<< (std::ostream& o, const BleuStats& b) {
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    if (n>0) {
      o << "; ";
    }
    o << b.hits_[n] << "/" << b.tots_[n]; 
  }
  o << "; " << b.refLength_;
  return o;
}

BleuStats operator+ (const BleuStats& bs1, const BleuStats& bs2) {
  BleuStats bs;
  bs.refLength_ = bs1.refLength_ + bs2.refLength_;
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    bs.hits_[n] = bs1.hits_[n] + bs2.hits_[n];
    bs.tots_[n] = bs1.tots_[n] + bs2.tots_[n];
  }
  return bs;
}

BleuStats operator- (const BleuStats& bs1, const BleuStats& bs2) {
  BleuStats bs;
  bs.refLength_ = bs1.refLength_ - bs2.refLength_;
  for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
    bs.tots_[n] = bs1.tots_[n] - bs2.tots_[n];
    bs.hits_[n] = bs1.hits_[n] - bs2.hits_[n];
  }
  return bs;
}

class BleuScorer {
public:
  typedef std::vector<Wid> NGram;
  typedef unordered_map<NGram, unsigned int, hashfvecint64, hasheqvecint64> NGramToCountMap;
  vector<NGramToCountMap> refCounts;
  vector<unsigned int> refLengths;

  BleuScorer(ucam::util::RegistryPO const& rg) {
    std::string refFile(rg.get<std::string>("refs"));
    std::cerr << "Processing file " << refFile << std::endl;
    std::ifstream ifs(refFile.c_str());
    std::string line;
    while (getline(ifs, line)) {
      std::istringstream iss(line);
      SentenceIdx sidx;
      Wid w;
      while (iss >> w)
	sidx.push_back(w);
      refLengths.push_back(sidx.size());
      refCounts.push_back(ScanNGrams(sidx));
    }
    ifs.close();
    std::cerr << "refLengths.size() " << refLengths.size() << std::endl;
    std::cerr << "refCounts.size() " << refCounts.size() << std::endl;
  }

  unsigned int ClosestReferenceLength (Sid sid, const unsigned int hypLength) const {
    return refLengths[sid];
  }

  BleuStats SentenceBleuStats (const Sid sid, const SentenceIdx& hyp) {
    BleuStats bs;
    bs.refLength_ = ClosestReferenceLength (sid, hyp.size());
    for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER && n < hyp.size(); ++n) {
      NGramToCountMap hypCounts;
      if (hyp.size() > n) {
	bs.tots_[n] = hyp.size() - n;
      }
      for (unsigned int i = 0; i < hyp.size() - n; ++i) {
	hypCounts[SubStr (hyp, i, n)]++;
      }
      for (NGramToCountMap::const_iterator hit = hypCounts.begin(); hit != hypCounts.end(); ++hit) {
	NGramToCountMap::const_iterator rit = refCounts[sid].find (hit->first);
	bs.hits_[n] += std::min (rit == refCounts[sid].end() ? 0 : rit->second, hit->second); // Sum clipped counts
      }
    }
    return bs;
  }

  Bleu ComputeBleu(const BleuStats& bs) {
    double logBleu = 0.0;
    double logBrev = 0.0;
    for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
      logBleu += std::log ( (double) bs.hits_[n] / (double) bs.tots_[n]);
    }
    logBleu *= 1 / (double) BleuStats::MAX_BLEU_ORDER;
    logBrev = std::min (0.0, 1 - bs.refLength_ / (double) (bs.tots_[0]) );
    return Bleu(exp(logBleu + logBrev), exp(logBrev));
  } 

  NGram SubStr (const SentenceIdx& s, const unsigned int n, const unsigned int l) const {
    return NGram (s.begin() + n, s.begin() + n + l + 1);
  }

  NGramToCountMap ScanNGrams(SentenceIdx ref) {
    NGramToCountMap m;
    for (unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n) {
      int refssize_minus_n = ref.size() - n; // needs to be an int, not a unsigned int in case it is negative
      for (int i = 0; i < refssize_minus_n; ++i) {
        NGram u = SubStr (ref, i, n);
	m[u]++;
      }
    }
    return m;
  }

};


#endif
