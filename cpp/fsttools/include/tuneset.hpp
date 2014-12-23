#ifndef LMERT_TUNESET_HPP
#define LMERT_TUNESET_HPP

#include <vector>

#include <constants-fsttools.hpp>
#include <fstutils.hpp>
#include <bleu.hpp>

namespace ucam {
namespace fsttools {

template <class Arc>
class TuneSet {
 public:
  typedef boost::shared_ptr<fst::VectorFst<Arc> > VectorFstPtr;
  typedef std::vector<VectorFstPtr> VectorFstPtrVector;
  Sid sidMax;
  VectorFstPtrVector cachedLats;

  // \todo: for general purpose, this should not depend on kInput or kRangeOne
  TuneSet ( ucam::util::RegistryPO const& rg ) {
    using namespace ucam::util;
    using namespace HifstConstants;
    using namespace fst;

    for ( IntRangePtr ir ( IntRangeFactory ( rg, kRangeOne ) );
          !ir->done(); ir->next() ) {
      PatternAddress<unsigned> input ( rg.get<string> ( kInput.c_str() ) );
      VectorFstPtr ifst ( VectorFstRead<Arc> ( input ( ir->get() ) ) );
      TopSort ( &*ifst );
      cachedLats.push_back ( ifst );
    }
    sidMax = cachedLats.size();
    FORCELINFO ( "Loaded tuneset lattices: " << sidMax );
  }

  fst::VectorFst<Arc>* GetLattice ( Sid sid ) {
    if ( sid > cachedLats.size() ) {
      LERROR ( "Requested lattice not loaded" << sid );
      exit ( EXIT_FAILURE );
    }
    return &* ( cachedLats[sid] );
  }

  // compute bleu under vw vector weight
  Bleu ComputeBleu ( BleuScorer& bs ) {
    using namespace fst;
    BleuStats bstats;
    for ( int i = 0; i < sidMax; ++i ) {
      SentenceIdx h;
      FstGetBestHypothesis<Arc, Wid> ( *cachedLats[i], h);
      if ( h.size() > 2 ) { // remove <s> </s>
        h.erase ( h.begin() );
        h.pop_back();
      }
      bstats = bstats + bs.SentenceBleuStats ( i, h );
    }
    return bs.ComputeBleu ( bstats );
  };


  // \todo I think this method will only work with Arc=TupleArc32.
  // PARAMS32 temporary replacement should be done externally,
  // perhaps implemented in a semiring-specific wrapper
  Bleu ComputeBleu ( BleuScorer& bs, PARAMS32 const& vw ) {
    using namespace fst;
    PARAMS32 dval = TropicalSparseTupleWeight<float>::Params();
    TropicalSparseTupleWeight<float>::Params() = vw;
    BleuStats bstats;

    for ( int i = 0; i < sidMax; ++i ) {
      SentenceIdx h;
      FstGetBestHypothesis<Arc, Wid> ( *cachedLats[i], h);

      if ( h.size() > 2 ) { // remove <s> </s>
        h.erase ( h.begin() );
        h.pop_back();
      }
      // \todo define += operator?
      bstats = bstats + bs.SentenceBleuStats ( i, h );
    }
    TropicalSparseTupleWeight<float>::Params() = dval;
    return bs.ComputeBleu ( bstats );
  }

};

}}  // end namespaces

#endif
