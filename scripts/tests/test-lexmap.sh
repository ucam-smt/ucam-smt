#!/bin/bash

source runtests.sh

lexmap=$CAM_SMT_DIR/bin/lexmap.${TGTBINMK}.bin
range=1:2

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;


################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_lexmap_found() {

   if [ -e $lexmap ] ; then echo 1; return; fi
    echo 0
}

test_0002_lexmap_execute_std2lex(){

    $lexmap \
	--input=data/fsts/?.lat.fst.gz \
	--range=$range \
	--output=$BASEDIR/?.lex.fst \
	--action=std2lex &>/dev/null

    for k in 1 2; do
	fstprint $BASEDIR/$k.lex.fst | sed -e 's:,.*$::g' | fstcompile  | fstdeterminize > $BASEDIR/$k.std.det.fst
	fstprint $REFDIR/$k.lex.det.fst | sed -e 's:,.*$::g' | fstcompile > $BASEDIR/$k.std.det.ref.fst
	if fstequivalent $BASEDIR/$k.std.det.fst $BASEDIR/$k.std.det.ref.fst;  then echo -e ""; else echo 0; return ; fi

    done
    echo 1;

}

test_0003_lexmap_execute_lex2std(){
    $lexmap \
	--input=data/fsts/lex/?.fst.gz \
	--range=$range \
	--output=$BASEDIR/?.std.fst \
	--action=lex2std &>/dev/null

    for k in 1 2 ; do
	fstdeterminize $BASEDIR/$k.std.fst > $BASEDIR/$k.std.det.fst
	if fstequivalent $BASEDIR/$k.std.det.fst $REFDIR/$k.std.det.fst ; then echo -e ""; else echo 0; return ; fi
    done
    echo 1;

}

test_0004_lexmap_execute_projectweight2(){
    $lexmap \
	--input=data/fsts/lex/?.fst.gz \
	--range=$range \
	--output=$BASEDIR/?.pw2.fst \
	--action=projectweight2 &>/dev/null

    for k in 1 2 ; do

	fstprint $BASEDIR/$k.pw2.fst | sed -e 's:,.*$::g' | fstcompile | fstdeterminize > $BASEDIR/$k.pw2.std.det.fst
	fstprint $REFDIR/$k.pw2.det.fst | sed -e 's:,.*$::g' | fstcompile > $BASEDIR/$k.pw2.std.det.ref.fst
	if fstequivalent $BASEDIR/$k.pw2.std.det.fst $BASEDIR/$k.pw2.std.det.ref.fst ; then echo -e ""; else echo 0; return ; fi
    done
    echo 1;

}

################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES
runtests



