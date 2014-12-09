#pragma once


namespace ucam {
namespace util {

struct MainClass {
  typedef util::RegistryPO RegistryPO;
  boost::scoped_ptr<RegistryPO> rg_;
  const char **argv_;
  MainClass(int argc, const char *argv[])
      :argv_(argv)
  {
    rg_.reset(new RegistryPO(argc,argv));
    util::initLogger ( argc, argv );
    FORCELINFO ( argv[0] << " starts!" );
    FORCELINFO ( rg_->dump ( "CONFIG parameters:\n====================="
                             , "=====================" ) );

  }

  // To be implemented by each tool;
  void run();

  ~MainClass() {
    FORCELINFO ( argv_[0] << " ends!" );
  }

};

}} // end namespaces

/**
 * \brief Main function.
 * \param       argc: Number of command-line program options.
 * \param       argv: Actual program options.
 * \remarks     Main function to be used by any tool -- see MainClass.
 * First parses program options with boost, then loads and chains several task classes.
 * Finally, kick off translation for a range of sentences.
 */
int main ( int argc, const char *argv[] ) {
  (ucam::util::MainClass(argc,argv).run());
  return 0;
}



