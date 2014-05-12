Cambridge SMT
=============

This package contains the Cambridge SMT translation system.

GETTING STARTED
 + Requirements: You need to install OpenFST 1.3.*, boost_1_53_0+, and google tests 1.4.0+. 
   We have compiled succesfully with g++ 4.2.1 to 4.6.1.
   Bash environment assumed.
   For more details, i.e. particular extensions to these libraries, see externals/get-build-libraries.sh. Note that build-test.sh
   can try to download them for you, and install them on externals directory.
 + STEP 1 (optional): Create/edit Makefile.inc (see Makefile.inc.TEMPLATE) and update environment variables pointing to these packages.
 + STEP 2: run build-test.sh -- should install all binaries and run tests succesfully out-of-the-box (to re-run tests, see tests.sh).
 + STEP 3: If you have doxygen, you can build your documentation in doc directory. 

LICENSING
 See COPYING file for more details.

CONTENTS DESCRIPTION 
+ bin/ : binaries and libraries (after installing)
+ doc/: doxygen documentation. 
+ cpp/: All cpp code
+ cpp/fsttools   : code for several basic tools 
+ cpp/hifst      : hifst-related binaries 
+ cpp/latmert    : lattice-mert related binaries
+ cpp/tests      : Unit testing
+ scripts	 : Any auxiliary scripts (bash, pl,...)
+ scripts/tests/ : Script testing of installed binaries
+ externals      : contains KenLM (http://kheafield.com/code/kenlm/)


TODO LIST:
- Use cmake
- (De-)Tokenizer based on regular expressions, read configuration from user files.
- Chop grammars: it is a known issue that memory footprint can increase if cell lattices are not deleted asap. 
  This can be dealt with automatically by precounting number of times each cell will be visited 
- Keep track of word alignment in decoding
- Decoding directly with sparse semiring


