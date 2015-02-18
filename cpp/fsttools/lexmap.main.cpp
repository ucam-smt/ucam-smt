/**
 * \file
 * \brief Main file for applylm tool
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#include <main.custom_assert.hpp>
#include <main.logger.hpp>
#include <main.lexmap.hpp>

/*
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks
 */

template <class ArcT
          , class Arc2T
          , class MapperT
          , class WeightFunctorT >
int run ( ucam::util::RegistryPO const& rg) {
  ucam::util::PatternAddress<unsigned> input (rg.get<std::string>
      (HifstConstants::kInput) );
  ucam::util::PatternAddress<unsigned> output (rg.get<std::string>
      (HifstConstants::kOutput) );
  WeightFunctorT mwcopy;
  for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg,
                                    HifstConstants::kRangeOne ) );
        !ir->done();
        ir->next() ) {
    FORCELINFO ("Processing file " << input ( ir->get() ) );
    boost::scoped_ptr< fst::VectorFst<ArcT> > ifst (fst::VectorFstRead<ArcT>
        ( input (
            ir->get() ) ) );
    boost::scoped_ptr< fst::VectorFst<Arc2T> > ofst (new fst::VectorFst<Arc2T>);
    Map ( *ifst, &*ofst, MapperT ( mwcopy ) );
    fst::FstWrite<Arc2T> ( *ofst, output (ir->get() ) );
  }
};

int main ( int argc, const char* argv[] ) {
  ucam::util::initLogger ( argc, argv );
  FORCELINFO ( argv[0] << " starts!" );
  ucam::util::RegistryPO rg ( argc, argv );
  FORCELINFO ( rg.dump ( "CONFIG parameters:\n=====================",
                         "=====================" ) );
  if (rg.get<std::string> (HifstConstants::kAction) ==
      HifstConstants::kActionProjectweight2 ) {
    run<fst::LexStdArc
    , fst::LexStdArc
    ,  fst::GenericWeightMapper<fst::LexStdArc, fst::LexStdArc, fst::MakeWeight2<fst::LexStdArc> >,  fst::MakeWeight2<fst::LexStdArc>
    > (rg);
  } else if (rg.get<std::string> (HifstConstants::kAction) ==
             HifstConstants::kActionLex2std) {
    run<fst::LexStdArc
    , fst::StdArc
    ,  fst::GenericWeightMapper<fst::LexStdArc, fst::StdArc, fst::LexToStd >,  fst::LexToStd
    > (rg);
  } else if (rg.get<std::string> (HifstConstants::kAction) ==
             HifstConstants::kActionStd2lex) {
    run<fst::StdArc
    , fst::LexStdArc
    ,  fst::GenericWeightMapper<fst::StdArc, fst::LexStdArc, fst::MakeWeight2<fst::LexStdArc> >,  fst::MakeWeight2<fst::LexStdArc>
    > (rg);
  } else {
    LERROR ("Action not recognized! Check program option.");
  }
  FORCELINFO ( argv[0] << " finished!" );
}
