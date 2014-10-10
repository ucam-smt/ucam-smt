#ifndef FSTTOOLS_BLEU_HPP
#define FSTTOOLS_BLEU_HPP

#include <iostream>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <tr1/unordered_map>
#include <boost/functional/hash.hpp>

typedef boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_sink> pipe_out;
typedef boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_source> pipe_in;

namespace ucam {
namespace fsttools {

// parameter vector
typedef std::vector<float> PARAMS32;
typedef unsigned Sid;
// word and sentence typedefs
typedef long long Wid;
typedef std::vector<Wid> SentenceIdx;


class Bleu {
 public:
  Bleu ( const double bleu = -std::numeric_limits<double>::infinity(),
         const double brev = 0 ) : m_bleu ( bleu ), m_brev ( brev ) {}
  inline double GetError() const {
    return m_bleu;
  }

  double m_bleu;
  double m_brev;
};

std::ostream& operator<< ( std::ostream& o, const Bleu& b ) {
  o << b.m_bleu << " (" << b.m_brev << ")";
  return o;
}

bool operator> ( Bleu& b1, Bleu& b2 ) {
  return b1.m_bleu > b2.m_bleu;
}

class BleuStats {
 public:
  static const unsigned int MAX_BLEU_ORDER = 4;
	std::vector<int> tots_; // order-specific ngram totals
	std::vector<int> hits_; // order-specific ngram hits
  long refLength_; // effective reference length

  BleuStats () : refLength_ ( 0 ) {
    tots_.resize ( MAX_BLEU_ORDER, 0 );
    hits_.resize ( MAX_BLEU_ORDER, 0 );
  }

  BleuStats ( std::vector<int> const &tots
              , std::vector<int> const &hits
              , long const refLength ) :
    tots_ ( tots ),
    hits_ ( hits ),
    refLength_ ( refLength )
  {}
};

std::ostream& operator<< ( std::ostream& o, const BleuStats& b ) {
  for ( unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n ) {
    if ( n > 0 ) {
      o << "; ";
    }
    o << b.hits_[n] << "/" << b.tots_[n];
  }
  o << "; " << b.refLength_;
  return o;
}

BleuStats operator+ ( const BleuStats& bs1, const BleuStats& bs2 ) {
  BleuStats bs;
  bs.refLength_ = bs1.refLength_ + bs2.refLength_;

  for ( unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n ) {
    bs.hits_[n] = bs1.hits_[n] + bs2.hits_[n];
    bs.tots_[n] = bs1.tots_[n] + bs2.tots_[n];
  }
  return bs;
}

BleuStats operator- ( const BleuStats& bs1, const BleuStats& bs2 ) {
  BleuStats bs;
  bs.refLength_ = bs1.refLength_ - bs2.refLength_;

  for ( unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n ) {
    bs.tots_[n] = bs1.tots_[n] - bs2.tots_[n];
    bs.hits_[n] = bs1.hits_[n] - bs2.hits_[n];
  }
  return bs;
}

class LRUCache {
  typedef std::pair< SentenceIdx, BleuStats > EntryPair;
  typedef std::list< EntryPair > CacheList;
  typedef unordered_map< SentenceIdx, CacheList::iterator, boost::hash<SentenceIdx> > CacheMap;

public:
  LRUCache(unsigned int cacheSize=10000): cacheSize_(cacheSize), entries(0){};

  void insert(const SentenceIdx& hyp, const BleuStats& bs) {
    // push it to the front;
    cacheList.push_front(std::make_pair(hyp, bs));
    // add it to the cache map
    cacheMap[hyp] = cacheList.begin();
    // increase count of entries
    entries++;
    // check if it's time to remove the last element
    if (entries > cacheSize_) {
      // erase from the map the last cache list element
      cacheMap.erase(cacheList.back().first);
      // erase it from the list
      cacheList.pop_back();
      // decrease count
      entries--;
    }
  }

  bool contains(const SentenceIdx& hyp) const {
    CacheMap::const_iterator it = cacheMap.find(hyp);
    return it != cacheMap.end();
  }

  bool get(const SentenceIdx& hyp, BleuStats& bs) {
    CacheMap::const_iterator it = cacheMap.find(hyp);
    if (it == cacheMap.end()) {
      return false;
    }
    EntryPair entry = *(it->second);
    cacheList.erase(it->second);
    cacheList.push_front(entry);
    cacheMap[hyp] = cacheList.begin();
    bs = entry.second;
    return true;
  }

  BleuStats get(const SentenceIdx& hyp) {
    CacheMap::const_iterator it = cacheMap.find(hyp);
    if (it == cacheMap.end()) {
      BleuStats zero;
      return zero;
    }
    EntryPair entry = *(it->second);
    cacheList.erase(it->second);
    cacheList.push_front(entry);
    cacheMap[hyp] = cacheList.begin();
    return entry.second;
  }

private:
  unsigned int cacheSize_;
  unsigned int entries;
  CacheList cacheList;
  CacheMap cacheMap;
};


class BleuScorer {
public:
  typedef std::vector<Wid> NGram;
  typedef unordered_map<NGram, unsigned int,
			ucam::util::hashfvecint64, 
			ucam::util::hasheqvecint64> NGramToCountMap;
  vector<NGramToCountMap> refCounts;
  vector<unsigned int> refLengths;

  BleuScorer(std::string const & refFile, std::string const & extTokCmd, 
	     unsigned int cacheSize) : chits_(0), cmisses_(0){
    externalTokenizer_ = (extTokCmd.size() > 0);
    useCache_ = (cacheSize > 0);

    // -- pipez
    if (externalTokenizer_) {
      FORCELINFO("External tokenizer");
      int fd[2];
      OpenPipe(fd, extTokCmd);
      pipe_in *pIn = new pipe_in();
      pipe_out *pOut = new pipe_out();
      pOut->open( boost::iostreams::file_descriptor_sink(fd[0],
							 boost::iostreams::close_handle));
      pIn->open( boost::iostreams::file_descriptor_source(fd[1],
							  boost::iostreams::close_handle));
      normalIn = new std::istream(pIn);
      intOut = new std::ostream(pOut);
      oovId_ = 0;
    } else {
      FORCELINFO("No external tokenizer");
    }

    FORCELINFO("Processing file " << refFile );
    std::ifstream ifs ( refFile.c_str() );
    std::string line;
    int nr = 0;
    while ( getline ( ifs, line ) ) {
      SentenceIdx sidx;
      if (externalTokenizer_)
	sidx = ExternalRefTokenizer( line );
      else
	sidx = StringToSentenceIdx( line );
      refLengths.push_back ( sidx.size() );
      refCounts.push_back ( ScanNGrams ( sidx ) );
      nr++;
    }
    ifs.close();
    FORCELINFO("refLengths.size() " << refLengths.size());
    FORCELINFO("refCounts.size() " << refCounts.size() );
    if (useCache_) {
      bleuStatsCache.resize( nr );
      FORCELINFO("bleu stats cache size: " << cacheSize << " x " << nr);
    }
  }

  string CacheStats() {
    std::ostringstream os;
    os << "BleuStats Cache Stats: Cache Hits=" << chits_ << "; Cache Misses=" << cmisses_ << "; Rate=";
    os.precision(3);
    os << (float) chits_ / (float) (chits_ + cmisses_);
    return os.str();
  }

  unsigned int ClosestReferenceLength ( Sid sid,
					const unsigned int hypLength ) const {
    return refLengths[sid];
  }

  BleuStats SentenceBleuStats ( const Sid sid, const SentenceIdx& hypIdx ) {
    BleuStats bs;
    if (useCache_ && bleuStatsCache[sid].get(hypIdx, bs)) {
      chits_++;
      return bs;
    }
    cmisses_++;
    SentenceIdx hyp = (externalTokenizer_) ? ExternalHypTokenizer(hypIdx) : hypIdx;
    bs.refLength_ = ClosestReferenceLength ( sid, hyp.size() );
    for ( unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER
	    && n < hyp.size(); ++n ) {
      NGramToCountMap hypCounts;
      if ( hyp.size() > n ) {
        bs.tots_[n] = hyp.size() - n;
      }
      for ( unsigned int i = 0; i < hyp.size() - n; ++i ) {
        hypCounts[SubStr ( hyp, i, n )]++;
      }
      for ( NGramToCountMap::const_iterator hit = hypCounts.begin();
            hit != hypCounts.end(); ++hit ) {
        NGramToCountMap::const_iterator rit = refCounts[sid].find ( hit->first );
        bs.hits_[n] += std::min ( rit == refCounts[sid].end() ? 0 : rit->second,
                                  hit->second ); // Sum clipped counts
      }
    }
    if (useCache_)
      bleuStatsCache[sid].insert(hypIdx, bs);
    return bs;
  }

  Bleu ComputeBleu ( const BleuStats& bs ) {
    double logBleu = 0.0;
    double logBrev = 0.0;

    for ( unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n ) {
      logBleu += std::log ( ( double ) bs.hits_[n] / ( double ) bs.tots_[n] );
    }
    logBleu *= 1 / ( double ) BleuStats::MAX_BLEU_ORDER;
    logBrev = std::min ( 0.0, 1 - bs.refLength_ / ( double ) ( bs.tots_[0] ) );
    return Bleu ( exp ( logBleu + logBrev ), exp ( logBrev ) );
  }

  NGram SubStr ( const SentenceIdx& s, const unsigned int n,
                 const unsigned int l ) const {
    return NGram ( s.begin() + n, s.begin() + n + l + 1 );
  }

  NGramToCountMap ScanNGrams ( SentenceIdx const &ref ) const {
    NGramToCountMap m;

    for ( unsigned int n = 0; n < BleuStats::MAX_BLEU_ORDER; ++n ) {
      // needs to be an int, not a unsigned int in case it is negative
      int refssize_minus_n = ref.size() -  n; 
      for ( int i = 0; i < refssize_minus_n; ++i ) {
        NGram u = SubStr ( ref, i, n );
        ++m[u];
      }
    }
    return m;
  }
 

private:
  unordered_map<std::string, Wid> refWordMap_;
  Wid oovId_;
  bool externalTokenizer_;
  boost::mutex mutex;
  vector< LRUCache > bleuStatsCache;
  unsigned int chits_;
  unsigned int cmisses_;
  bool useCache_;

  // n.b. not to be multithreaded - references are loaded only once by main
  SentenceIdx ExternalRefTokenizer(std::string const &s) {
    (*intOut) << s << endl;
    string si;
    getline(*normalIn, si);
    std::istringstream is(si);
    SentenceIdx rs;
    string w;
    while (is >> w) {
      unordered_map<std::string, Wid>::iterator it = refWordMap_.find(w);
      if (it == refWordMap_.end()) {
	rs.push_back(oovId_);
	refWordMap_[w] = oovId_++;
      }	else {
	rs.push_back(it->second);
      }
    }
    return rs;
  }

  // returns wordid if word is in references; oov otherwise
  // ok for threading
  SentenceIdx ExternalHypTokenizer(SentenceIdx const &s) {
    if (s.size() == 0)
      return s;
    std::ostringstream os;
    os << s[0];
    for (int i=1; i<s.size(); i++)
      os << " " << s[i];
    string si;
    mutex.lock();
    (*intOut) << os.str() << endl;
    getline(*normalIn, si);
    mutex.unlock();
    std::istringstream is(si);
    SentenceIdx rs;
    string w;
    while (is >> w) {
      unordered_map<std::string, Wid>::iterator it = refWordMap_.find(w);
      if (it == refWordMap_.end()) {
	rs.push_back(oovId_);
      }	else {
	rs.push_back(it->second);
      }
    }
    return rs;
  }

  SentenceIdx StringToSentenceIdx(std::string const & s) {
    SentenceIdx rv;
    std::istringstream is(s);
    Wid w;
    while (is >> w)
      rv.push_back(w);
    return rv;
  }

  void OpenPipe(int *fd, string command) {
    int outfd[2];
    int infd[2];
    int oldstdin, oldstdout;
    int rs1 = pipe(outfd); // Where the parent is going to write to
    int rs2 = pipe(infd); // From where parent is going to read
    oldstdin = dup(0); // Save current stdin
    oldstdout = dup(1); // Save stdout
    close(0);
    close(1);
    dup2(outfd[0], 0); // Make the read end of outfd pipe as stdin
    dup2(infd[1], 1); // Make the write end of infd as stdout
    if (!fork()) {
      close(outfd[0]); // Not required for the child
      close(outfd[1]);
      close(infd[0]);
      close(infd[1]);
      char * cstr = new char[command.size() + 1];
      strcpy(cstr, command.c_str());
      char * const argv[] = { (char *) "/bin/bash", (char *) "-c", cstr, (char *) 0 };
      int err = execv(argv[0], argv);
      dup2(oldstdout, 1);
      FORCELINFO("Cannot run command: " << command);
      exit(1);
    } else {
      close(0); // Restore the original std fds of parent
      close(1);
      dup2(oldstdin, 0);
      dup2(oldstdout, 1);
      close(outfd[0]); // These are being used by the child
      close(infd[1]);
      fd[0] = outfd[1];
      fd[1] = infd[0];
    }
  }

  std::istream* normalIn;
  std::ostream* intOut;
};

}
} // end namespaces
#endif
