#!/bin/bash

source runtests.sh

lmbr=$CAM_SMT_DIR/bin/lmbr.${TGTBINMK}.bin
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;



################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_lmbr_found() {

   if [ -e $lmbr ] ; then echo 1; return; fi
    echo 0
}


test_0002_lmbr_execute(){

    $lmbr \
        --range=$range\
        --load.evidencespace=data/fsts/?.lat.fst.gz \
        --writeonebest=$BASEDIR/lmbr/%%alpha%%_%%wps%%.hyp \
        --alpha=0.4:0.1:0.5   \
        --wps=-0.01:0.02:0.01\
        --p=0.7410\
        --r=0.6200\
        --preprune=5  &> /dev/null

    for file in `ls $REFDIR/lmbr/*.hyp`; do if diff $file $BASEDIR/lmbr/`basename $file` ; then echo ""; else echo 0; return;  fi ; done

    echo 1
    return

}



################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



