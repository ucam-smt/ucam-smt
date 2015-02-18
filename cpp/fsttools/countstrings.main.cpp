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

#include <main.countstrings.hpp>
#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <boost/multiprecision/cpp_int.hpp>

//assumes topsorted fst
template<class Arc ,
         typename IntegerT >
IntegerT countstrings (fst::VectorFst<Arc>& myfst) {
  std::vector<IntegerT> counts;
  IntegerT cumcounts = 0;
  counts.assign (myfst.NumStates(), 0);
  counts[0] = 1;
  typedef typename Arc::StateId StateId;
  for (fst::StateIterator< fst::VectorFst<Arc> > si (myfst);
       !si.Done();
       si.Next() ) {
    typename Arc::StateId state_id = si.Value();
    if (myfst.Final (state_id) != Arc::Weight::Zero() ) {
      cumcounts += counts[state_id];
    }
    for (fst::MutableArcIterator< fst::MutableFst<Arc> > ai (&myfst, si.Value() );
         !ai.Done();
         ai.Next() ) {
      Arc arc = ai.Value();
      counts[arc.nextstate] += counts[state_id];
    }
  }
  return cumcounts;
};

template<class Arc>
void run(ucam::util::RegistryPO const &rg) {

  ucam::util::PatternAddress<unsigned> pi (rg.get<std::string>
      (HifstConstants::kInput) );
  ucam::util::PatternAddress<unsigned> po (rg.get<std::string>
      (HifstConstants::kOutput) );
  for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg,
                                    HifstConstants::kRangeOne ) );
        !ir->done();
        ir->next() ) {
    fst::VectorFst<Arc> *mfst = fst::VectorFstRead<Arc> (pi (
                                          ir->get() ) );
    if (!mfst) {
      LERROR("Could not read file:" << ir->get());
      exit(EXIT_FAILURE);
    }
    TopSort (mfst);
    boost::multiprecision::uint128_t j =
      countstrings<Arc, boost::multiprecision::uint128_t> (*mfst);
    std::stringstream ss;
    ss << j;
    ucam::util::oszfstream o (po (ir->get() ), true);
    o << ss.str()  << std::endl;
    LINFO ( pi (ir->get() ) << ":" << ss.str() ) ;
    o.close();
    delete mfst;
  }
}


int main (int argc,  const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "=====================" ) )  ;

  std::string semiring = rg.get<std::string> (HifstConstants::kHifstSemiring);
  if (semiring == HifstConstants::kHifstSemiringStdArc) {
    run<fst::StdArc> (rg);
  } else if (semiring == HifstConstants::kHifstSemiringLexStdArc) {
    run<fst::LexStdArc> (rg);
  } else if (semiring == HifstConstants::kHifstSemiringTupleArc) {
    run<TupleArc32> (rg);
  } else {
    LERROR ("Sorry, semiring option not correctly defined");
  }
  FORCELINFO ( argv[0] << " ends!" );
}
