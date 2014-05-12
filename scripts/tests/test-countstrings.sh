#!/bin/bash

source runtests.sh

countstrings=$CAM_SMT_DIR/bin/countstrings.${TGTBINMK}.bin
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;


################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_countstrings_found() {

   if [ -e $countstrings ] ; then echo 1; return; fi
    echo 0
}


test_0002_countstrings_execute(){

    $countstrings \
	--input=data/fsts/?.lat.fst.gz \
	--range=$range \
	--output=$BASEDIR/counts  &>/dev/null

    if diff $BASEDIR/counts $REFDIR/counts ; then echo 1; return ; fi
    echo 0;

}


################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



