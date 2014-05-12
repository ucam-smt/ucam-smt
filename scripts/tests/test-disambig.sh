#!/bin/bash

source runtests.sh

disambig=$CAM_SMT_DIR/bin/disambig.${TGTBINMK}.bin
range=1:2

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_disambig_found() {

   if [ -e $disambig ] ; then echo 1; return; fi
    echo 0
}

test_0002_disambig_execute(){

    $disambig \
	--recaser.lm.load=data/lm/lm.tc.gz \
	--recaser.unimap.load=data/rules/tc.unimap \
	--recaser.input=data/fsts/tc/?.fst \
	--recaser.output=$BASEDIR/?.fst \
	--range=$range --semiring=lexstdarc \
	&>/dev/null
    for k in 1 2; do
	cat $BASEDIR/$k.fst | fstproject --project_output >  $BASEDIR/$k.o.fst
	if fstequivalent $BASEDIR/$k.o.fst $REFDIR/$k.o.fst; then echo -e ""; else echo 0; return; fi;

    done

    echo 1;
}

################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



