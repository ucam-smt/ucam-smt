Cambridge SMT
=============

This package contains the Cambridge SMT system.

GETTING STARTED
 + Requirements: You need to install the following libraries: OpenFST 1.3.*, boost_1_53_0+, and google tests 1.4.0+. 
   Should compile succesfully with g++ versions 4.2.1 to 4.6.1. Also required java 1.7+ and sbt (see java/ruleXtract/README.md).
   Bash environment assumed.
   For more details, i.e. particular extensions/packages these libraries need to be installed with,
   see externals/get-build-libraries.sh. Note that build-test.sh can try to download them for you,
   and install them on externals directory.
 + STEP 1 (optional): Create/edit Makefile.inc (see Makefile.inc.TEMPLATE) and update environment variables pointing to these packages.
 + STEP 2: run build-test.sh -- should install all binaries and run tests succesfully out-of-the-box (to re-run tests, see tests.sh).
 + STEP 3: If you have doxygen, you can build your documentation in doc directory. 


Note that our lattice mert implementation (cpp/latmert) speeds up quite considerably if compiled with google perftools 
(https://code.google.com/p/gperftools/) and unwind library. We compile succesfully with gperftools 2.0 and libunwind 1.0.1. 
Once you have them installed, you just need to set up a couple of variables in Makefile.inc. Please see Makefile.inc for instructions.


LICENSING
 See COPYING file for more details.

CONTENTS DESCRIPTION 
+ doc/: doxygen documentation (if you run doxygen). 
+ cpp/: All c++ code
+ cpp/fsttools   : code for several basic fst tools 
+ cpp/hifst      : hifst-related binaries 
+ cpp/latmert    : lattice-mert related binaries
+ cpp/tests      : Unit testing
+ java/          : All java code
+ java/ruleXtract: Tool for rule extraction
+ scripts	 : Auxiliary scripts (bash, pl,...)
+ scripts/tests/ : Regression testing
+ externals      : contains KenLM (http://kheafield.com/code/kenlm/)



WISH LIST:
The following is a wish list of things that might (or not) make it into
the package at some point.

- Use cmake
- (De-)Tokenizer based on regular expressions, read language-specific configuration from user files.
- Chop grammars: it is a known issue that memory footprint can increase for certain sentences 
  if cell lattices are not deleted asap. 
  This can be dealt with automatically by precounting number of times each cell will be visited 
- Keep track of word alignment in decoding
- Decoding directly with the tropical sparse vector weight semiring
- Deal with integer mapping more transparently.
