#!bin/bash

export CXXFLAGS="-DHAVE_ZLIB "
pushd kenlm; 
rm {lm,util}/*.o 2>/dev/null
set -e

CXX=${CXX:-g++}

CXXFLAGS+=" -I. -O3 -DNDEBUG -DKENLM_MAX_ORDER=6"

##If this fails for you, consider using bjam.
#if [ ${#NPLM} != 0 ]; then
#    CXXFLAGS+=" -DHAVE_NPLM -lneuralLM -L$NPLM/src -I$NPLM/src -lboost_thread-mt -fopenmp"
#    ADDED_PATHS="lm/wrappers/*.cc"
#fi
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
mkdir -p ../bin; ar rcs ../bin/libkenlm.a kenlm/lm/*.o kenlm/util/*.o kenlm/util/*/*.o