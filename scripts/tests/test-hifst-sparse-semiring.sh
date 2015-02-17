#!/bin/bash

source runtests.sh
hifst=$CAM_SMT_DIR/bin/hifst.${TGTBINMK}.bin
rules2weights=$CAM_SMT_DIR/bin/rules2weights.${TGTBINMK}.bin
grammar=data/rules/trivial.grammar 
tstidx=data/source.text
languagemodel=data/lm/trivial.lm.gz
languagemodeltrie=data/lm/trivial.lm.trie.mmap
weaklanguagemodel=data/lm/trivial.lm.gz
wlanguagemodel=data/lm/trivial.lm.words.gz
wweaklanguagemodel=data/lm/trivial.lm.words.gz
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

export PATH=$OPENFST_BIN:$PATH
################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_hifst_found(){
    if [ ! -f $hifst ]; then echo 0 ; return; fi
    echo 1
}
test_0002_rules2weights_found() {
    if [ ! -f $rules2weights ]; then echo 0 ; return; fi
    echo 1
}


test_0003_fstprint_found(){
    if [ ! `which fstprint` ]; then echo 0 ; return; fi
    echo 1
}


test_0004_lmfile_exists(){
    if [ ! -f $languagemodel ] ; then echo 0; return; fi
    echo 1;
}

test_0005_grammar_exists(){
    if [ ! -f $grammar ] ; then echo 0; return; fi
    echo 1;
}

test_0006_testset_exists(){
    if [ ! -f $tstidx ] ; then echo 0; return; fi
    echo 1;
}

test_0007_lextropicalso_exists(){
    if [ ! -f $CAM_SMT_DIR/bin/tropical_LT_tropical-arc.so   ] ; then echo 0 ; return; fi
    echo 1
}

test_0008_tropicalsparsetupleso_exists(){
    if [ ! -f $CAM_SMT_DIR/bin/tropicalsparsetuple-arc.so ] ; then echo 0; return; fi;
    echo 1;
}

test_0009_md5sum_found(){
    if [ "`which md5sum`" == "" ]; then echo 0 ; return; fi
    echo 1
}


test_0010_translate_sparse() {

    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats/?.fst.gz  \
        --lm.load=$languagemodel  \
        --lm.featureweights=0.5 \
        --hifst.prune=9  \
        --semiring=tuplearc &>/dev/null

     seqrange=`echo $range | sed -e 's:\:: :g'`
     for k in `seq $seqrange`; do
         if [ "`zcat $BASEDIR/lats/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
         mkdir -p tmp; zcat $BASEDIR/lats/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;
         if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;
     done

###Success
    echo 1
}


#### Rule indices are passed simultaneously as input and features
test_0011_align_sparse() {

        $hifst \
            --grammar.load=$grammar \
            --source.load=$tstidx  \
            --referencefilter.load=$BASEDIR/lats/?.fst.gz \
            --referencefilter.prunereferenceshortestpath=2 \
            --hifst.lattice.store=$BASEDIR/alilats/?.fst.gz  \
            --hifst.localprune.enable=yes  --hifst.localprune.conditions=X,1,1,9,M,1,1,9,V,1,1,9 \
            --hifst.prune=9  \
	    --hifst.alilatsmode=yes \
            --semiring=tuplearc   &>/dev/null


    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
 	mkdir -p tmp;
### Note: fstproject - openfst 1.3.1  fails
	zcat $BASEDIR/alilats/$k.fst.gz | fstproject | fstrmepsilon | fstdeterminize | fstminimize | fstrmepsilon > tmp/$k.test.fst;
	zcat $REFDIR/alilats/$k.fst.gz | fstproject | fstrmepsilon | fstdeterminize | fstminimize | fstrmepsilon > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;
    done

###Success
   echo 1
}


test_0012_translate_pdt_sparse() {

     $hifst --logger.verbose \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
	--hifst.lattice.store=$BASEDIR/pdt-lats/?.fst.gz  \
        --lm.load=$languagemodel \
	--hifst.replacefstbyarc.numstates=0 \
	--hifst.usepdt=yes --hifst.rtnopt=no --semiring=tuplearc  &>/dev/null
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
	if [ "`zcat $BASEDIR/pdt-lats/$k.fst.gz | fstinfo`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/pdt-lats/$k.fst.gz  | fstprint | sed -e s:,.*$::g | fstcompile \
       	    | pdtexpand --pdt_parentheses=$BASEDIR/pdt-lats/$k.fst.gz.parens  \
	    | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.test.fst;

	zcat $REFDIR/lats/$k.fst.gz  | fstprint | sed -e s:,.*$::g | fstcompile > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;
    done

###Success
    echo 1
}

test_0013_translate_pdt2_sparse() {
( #set -x
    $hifst --logger.verbose \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.usepdt=yes \
        --hifst.lattice.store=$BASEDIR/pdt-lats2/?.fst.gz  \
	--lm.featureweights=1 --lm.load=$languagemodel --lm.wps=0\
        --hifst.localprune.enable=yes --hifst.localprune.conditions=S,-1,1,9 --hifst.localprune.lm.featureweights=1 --hifst.localprune.lm.load=$weaklanguagemodel --hifst.localprune.lm.wps=-2.30 \
        --semiring=tuplearc &>/dev/null
)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/pdt-lats2/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/pdt-lats2/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 
    done

###Success
    echo 1

}

test_0014_convert_lats_to_veclats() {

( #set -x
    $rules2weights \
	--range=$range \
	--rulestoweights.loadgrammar=$grammar \
        --rulestoweights.loadalilats=$BASEDIR/lats/?.fst.gz \
 	--rulestoweights.store=$BASEDIR/vwlats/?.fst.gz --logger.verbose

) &>/dev/null


    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/vwlats/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/vwlats/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/vwlats/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 
    done

###Success
    echo 1 
}


test_0015_align_convert_lats_to_veclats() {

(# set -x
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats/?.fst.gz  \
        --lm.load=$languagemodel  \
        --lm.featureweights=0.5 \
        --hifst.prune=9  \
        --semiring=tuplearc --rulestoweights.store=$BASEDIR/vwlats2/?.fst.gz --rulestoweights.enable=yes
) &>/dev/null

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/vwlats2/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/vwlats2/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/vwlats/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 
    done

###Success
    echo 1 
}


test_0016_align_convert_lats_to_veclats_2lm() {

(# set -x
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats/?.fst.gz  \
        --lm.load=$languagemodel,$languagemodeltrie  \
        --lm.featureweights=0.25,0.25 --lm.wps=0,0 \
        --hifst.prune=9  \
        --semiring=tuplearc --rulestoweights.store=$BASEDIR/vwlats.0016/?.fst.gz --rulestoweights.enable=yes
) &>/dev/null

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/vwlats.0016/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/vwlats.0016/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/vwlats.0016/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 
    done

###Success
    echo 1 
}



################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests


