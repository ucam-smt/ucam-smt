#!/bin/bash

source runtests.sh

printstrings=$CAM_SMT_DIR/bin/printstrings.${TGTBINMK}.bin
range=1:2

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_printstrings_found() {

   if [ -e $printstrings ] ; then echo 1; return; fi
    echo 0
}

test_0002_printstrings_tropical(){

    mkdir -p tmp $BASEDIR
    $printstrings \
	--input=data/fsts/?.lat.fst.gz \
	--range=$range -u -w -n 1000 > $BASEDIR/output.0002 2>/dev/null

    if diff $BASEDIR/output.0002 $REFDIR/output.0002 ; then echo ; else echo 0; return ; fi
    echo 1
    return
}

test_0003_printstrings_lexstdarc(){

    mkdir -p tmp
    $printstrings \
	--input=data/fsts/lex/?.fst.gz \
	--range=1,2 --semiring=lexstdarc -u -w -n 1000 > $BASEDIR/output.0003 2> /dev/null

    if diff $BASEDIR/output.0003 $REFDIR/output.0003 ; then echo ; else echo 0; return ; fi

    echo 1
}

test_0004_printstrings_tuplearc(){

    mkdir -p tmp
    $printstrings \
	--input=data/fsts/vec/?.fst.gz \
	--range=1,2 --semiring=tuplearc -u -w -n 1000 \
	--tuplearc.weights=1.0,0.583849,0.980097,2.592360,-0.781944,0.016636,20.602014,-3.373619,-3.064969,0.727553,0.120692,0.331922 \
	> $BASEDIR/output.0004 2> /dev/null

    if diff $BASEDIR/output.0004 $REFDIR/output.0004 ; then echo ; else echo 0; return ; fi

    echo 1
}


# Also test multiple output files
test_0005_printstrings_tropical_multiple_files(){

    mkdir -p tmp $BASEDIR
    $printstrings \
	--input=data/fsts/?.lat.fst.gz \
	--range=$range -u -w -n 1000 \
	--output=$BASEDIR/output.0005.?.gz 2> /dev/null

    zcat $BASEDIR/output.0005.1.gz $BASEDIR/output.0005.2.gz > $BASEDIR/output.0005

    if diff $BASEDIR/output.0005 $REFDIR/output.0002 ; then echo ; else echo 0; return ; fi
    echo 1
    return
}


################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



