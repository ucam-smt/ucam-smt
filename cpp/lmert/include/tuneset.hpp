#ifndef TUNESET_HPP
#define TUNESET_HPP

template <class Arc>
class TuneSet {
public:

  TuneSet(ucam::util::RegistryPO const& rg) {
    for (ucam::util::IntRangePtr ir(ucam::util::IntRangeFactory(rg, HifstConstants::kRangeOne)); !ir->done(); ir->next() ) {
      using ucam::util::oszfstream;
      ucam::util::PatternAddress<unsigned> input (rg.get<std::string> (HifstConstants::kInput.c_str()));
      fst::VectorFst<Arc> *ifst = fst::VectorFstRead<Arc> (input(ir->get()));
      TopSort(ifst);
      cachedLats.push_back( ifst );
    }
    sidMax = cachedLats.size();
    FORCELINFO("Loaded tuneset lattices: " << sidMax);
  }

  Sid sidMax;

  ~TuneSet() {
    for (typename std::vector< fst::VectorFst<Arc> *>::iterator it = cachedLats.begin(); it != cachedLats.end(); it++) {
      delete *it;
    }
  }

  fst::VectorFst<Arc>* GetLattice(Sid sid) {
    if (sid > cachedLats.size())
      LERROR("Requested lattice not loaded" << sid); 
    return cachedLats[sid];
  }

  Bleu ComputeBleu(BleuScorer& bs, PARAMS32& vw) {
    PARAMS32 dval = fst::TropicalSparseTupleWeight<float>::Params();
    fst::TropicalSparseTupleWeight<float>::Params() = vw;
    BleuStats bstats;
    for (int i=0; i<sidMax; i++) {
      SentenceIdx h;
      fst::VectorFst<Arc> fst;
      fst::ShortestPath(*cachedLats[i], &fst, 1);
      fst::RmEpsilon(&fst);
      fst::TopSort(&fst);
      for ( fst::StateIterator< fst::VectorFst<Arc> > si ( fst ); !si.Done();  si.Next() ) {
	for ( fst::ArcIterator< fst::VectorFst<Arc> > ai ( fst, si.Value() );  !ai.Done(); ai.Next() ) {
	  h.push_back(ai.Value().ilabel);
	}
      }
      if (h.size() > 2) { // remove <s> </s>
	h.erase(h.begin());
	h.pop_back();
      }
      bstats = bstats + bs.SentenceBleuStats(i, h);
    }
    fst::TropicalSparseTupleWeight<float>::Params() = dval;
    return bs.ComputeBleu(bstats);
  }


  vector< fst::VectorFst<Arc> *> cachedLats;
};

#endif
