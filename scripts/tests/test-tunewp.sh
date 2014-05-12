#!/bin/bash

source runtests.sh

tunewp=$CAM_SMT_DIR/bin/tunewp.${TGTBINMK}.bin
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_tunewp_found() {

   if [ -e $tunewp ] ; then echo 1; return; fi

    echo 0
}


test_0002_tunewp_execute(){

    $tunewp \
	--input=REFFILES/test-hifst+alilats2splats/lats/?.fst.gz \
	--range=$range --semiring=lexstdarc \
	--output=$BASEDIR/wp_%%wp%%/?.fst \
	--word_penalty=100 &>/dev/null

    mkdir -p tmp; 
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/wp_100.00/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	if fstequivalent $BASEDIR/wp_100.00/$k.fst $REFDIR/wp_100.00/$k.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

    echo 1
}


################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



