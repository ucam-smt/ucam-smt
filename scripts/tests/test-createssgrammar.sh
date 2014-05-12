#!/bin/bash

source runtests.sh

createssgrammar=$CAM_SMT_DIR/bin/createssgrammar.${TGTBINMK}.bin
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;


################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order


test_0001_createssgrammar_found() {

   if [ -e $createssgrammar ] ; then echo 1; return; fi

    echo 0
}


test_0002_createssgrammar_execute(){

    $createssgrammar \
	--range=$range \
	--grammar.load=data/rules/trivial.grammar \
	--ssgrammar.store=$BASEDIR/?.gz \
	--source.load=data/source.text  &>/dev/null
    
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
	if diff $BASEDIR/$k.gz $REFDIR/$k.gz ; then echo ; else echo 0; return; fi
    done

# Ok!
    echo 1
}


################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



