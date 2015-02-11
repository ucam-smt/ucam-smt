#!bin/bash
. ../Makefile.inc
export CXXFLAGS="-DHAVE_ZLIB "
pushd kenlm; 
rm -f {lm,util}/*.o {lm,util}/*/*.o 
set -e # -x

CXX=${CXX:-g++}
CXXFLAGS+=" -I. -O3 -DNDEBUG -DKENLM_MAX_ORDER=6 "

##If this fails for you, consider using bjam.
if [ ${#NPLM_LIB} != 0 ]; then
    CXXFLAGS+=" -DHAVE_NPLM -lneuralLM -L$NPLM_LIB -I$NPLM_INCLUDE -I$BOOST_INCLUDE -lboost_thread -lboost_system -fopenmp"
    ADDED_PATHS="lm/wrappers/*.cc"
fi
echo 'Compiling with '$CXX $CXXFLAGS

#Grab all cc files in these directories except those ending in test.cc or main.cc
objects=""
for i in util/double-conversion/*.cc util/*.cc lm/*.cc $ADDED_PATHS; do
    if [ "${i%test.cc}" == "$i" ] && [ "${i%main.cc}" == "$i" ]; then
	$CXX $CXXFLAGS -c $i -o ${i%.cc}.o
	objects="$objects ${i%.cc}.o"
    fi
done

popd
mkdir -p ../bin; ar rcs ../bin/libkenlm.a `find kenlm/ | grep ".o$"`
