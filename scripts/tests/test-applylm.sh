#!/bin/bash

source runtests.sh

applylm=$CAM_SMT_DIR/bin/applylm.${TGTBINMK}.bin
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;


################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order


test_0001_applylm_found() {

   if [ -e $applylm ] ; then echo 1; return; fi
    echo 0
}


test_0002_applylm_execute(){

    $applylm \
	--range=$range \
	--lm.load=data/lm/trivial.lm.gz \
	--lm.featureweights=2 \
	--lattice.load=data/fsts/?.alilats.fst \
	--lattice.store=$BASEDIR/?.fst &> /dev/null

    mkdir -p tmp; 
    seqrange=`echo $range | sed -e 's:\:: :g'`
    LD_LIBRARY_PATH=$CAM_SMT_DIR/bin
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	fstproject --project_output $BASEDIR/$k.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.fst
	if fstequivalent tmp/$k.fst $REFDIR/$k.fst; then echo -e ""; else echo 0;  return; fi ; 	
    done

    echo 1
}


test_0003_applylm_lexstd_execute(){

    $applylm \
	--range=$range \
	--lm.load=data/lm/trivial.lm.gz \
	--semiring="lexstdarc" \
	--lattice.load=REFFILES/test-hifst+alilats2splats/lats/?.fst.gz \
	--lattice.store=$BASEDIR/?.lex.fst &> /dev/null

    mkdir -p tmp; 
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	fstproject --project_output $BASEDIR/$k.lex.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.lex.fst
	if fstequivalent tmp/$k.lex.fst $REFDIR/$k.lex.fst; then echo -e ""; else echo 0;  return; fi ; 	
    done

    echo 1
}



test_0004_applylm_lexstd_rmlmcost_execute(){

    $applylm \
	--range=$range \
	--lattice.load.deletelmcost \
	--lm.load=data/lm/trivial.lm.gz \
	--semiring="lexstdarc" \
	--lattice.load=REFFILES/test-hifst+alilats2splats/lats/?.fst.gz \
	--lattice.store=$BASEDIR/?.lex2.fst &> /dev/null

    mkdir -p tmp; 
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	fstproject --project_output $BASEDIR/$k.lex2.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.lex2.fst
	if fstequivalent tmp/$k.lex2.fst $REFDIR/$k.lex2.fst; then echo -e ""; else echo 0;  return; fi ; 	
    done

    echo 1
}


test_0005_applylm_wordslm_execute(){

    $applylm \
	--range=$range \
	--lm.load=data/lm/trivial.lm.words.gz \
	--lm.featureweights=2 \
	--lm.wordmap=data/sunmap \
	--lattice.load=data/fsts/?.alilats.fst \
	--lattice.store=$BASEDIR/?.word.fst &> /dev/null

    mkdir -p tmp; 
    seqrange=`echo $range | sed -e 's:\:: :g'`
    LD_LIBRARY_PATH=$CAM_SMT_DIR/bin
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	fstproject --project_output $BASEDIR/$k.word.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.fst
	if fstequivalent tmp/$k.fst $REFDIR/$k.fst; then echo -e ""; else echo 0;  return; fi ; 	
    done

    echo 1
}


test_0006_applylm_nplm_execute(){

    if [ -z $NPLM_LIB ]; then
	echo 1
	return
    fi
    $applylm \
	--range=$range \
	--lm.load=data/lm/inferno.nnlm \
	--lm.featureweights=2 \
	--lattice.load=data/fsts/?.alilats.fst \
	--lattice.store=$BASEDIR/nplm/?.fst &> /dev/null
    rm -Rf tmp
    mkdir -p tmp; 
    seqrange=`echo $range | sed -e 's:\:: :g'`
    LD_LIBRARY_PATH=$CAM_SMT_DIR/bin
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/nplm/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	fstproject --project_output $BASEDIR/nplm/$k.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.fst
	if fstequivalent tmp/$k.fst $REFDIR/nplm/$k.fst; then echo -e ""; else echo 0;  return; fi ; 	
    done

    echo 1
}


################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



