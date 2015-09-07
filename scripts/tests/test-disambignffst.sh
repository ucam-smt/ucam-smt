#!/bin/bash

source runtests.sh
disambignffst=$CAM_SMT_DIR/bin/disambignffst.${TGTBINMK}.bin
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME.
###################   They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other.
###################   The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_disambignffst_found() {

   if [ -e $disambignffst ] ; then echo 1; return; fi
    echo 0
}

# Tests disambiguation for a bunch of alignment lattices.
# Typically complex deterministic lattices map correctly
# after running first pass.
# But if it fails, setting --exit-on-first-pass-failure=no
# should get a correct result (will be slower, though).
test_0002_disambignffst_execute(){
    $disambignffst \
        --range=$range --determinize-output=yes \
        --input=data/fsts/bigger/?.fst.gz \
        --output=$BASEDIR/?.det.fst \
    &> /dev/null

    for file in `ls $REFDIR/?.det.fst`; do
        ref=`fstprint $file | md5sum | cut -f1 -d' ' `;
        tf=$BASEDIR/`basename $file`
        test=`fstprint $tf | md5sum | cut -f1 -d' '`
        if [ "$ref" != "$test" ]; then echo 0; fi
    done
    echo 1
}

test_0003_disambignffst_nthreads_execute(){
    $disambignffst \
        --nthreads=2 \
        --range=$range --determinize-output=yes \
        --input=data/fsts/bigger/?.fst.gz \
        --output=$BASEDIR/?.det.fst \
    &> /dev/null

    for file in `ls $REFDIR/?.det.fst`; do
        ref=`fstprint $file | md5sum | cut -f1 -d' ' `;
        tf=$BASEDIR/`basename $file`
        test=`fstprint $tf | md5sum | cut -f1 -d' '`
        if [ "$ref" != "$test" ]; then echo 0; fi
    done
    echo 1
}

################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES
runtests



