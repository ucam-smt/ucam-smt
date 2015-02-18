#!/bin/bash

source runtests.sh

hifst=$CAM_SMT_DIR/bin/hifst.${TGTBINMK}.bin
alilats2splats=$CAM_SMT_DIR/bin/alilats2splats.${TGTBINMK}.bin

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;


################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

### Testing a toy chop-grammar on a toy chop-sentence, chops marked by 0.
test_0001_hifst_chopgrammar(){

(  
    $hifst \
	--grammar.load=data/rules/trivial.grammar2 \
	--source.load=data/source2.text \
	--cykparser.ntexceptionsmaxspan=S,T,U,Q,R \
	--lm.load=data/lm/trivial.lm.gz  --hifst.prune=1 \
	--hifst.lattice.store=$BASEDIR/trans.fst  \
	--hifst.localprune.enable=yes --hifst.localprune.lm.load=data/lm/trivial.lm.gz --hifst.localprune.conditions=U,1,1,1,Q,1,1,1  \
	--stats.hifst.write=$BASEDIR/cykgrid --stats.hifst.cykgrid.enable=yes  &> /dev/null

)
    if diff $BASEDIR/cykgrid $REFDIR/cykgrid ; then echo ; else echo 0; return ; fi
    if fstequivalent $BASEDIR/trans.fst $REFDIR/trans.fst ; then echo ; else echo 0; return ; fi
   
# Ok!
    echo 1
}

### Testing a toy dependency grammar
test_0002_hifst_multiple_nt_grammar(){

(
    $hifst \
	--grammar.load=data/rules/trivial.grammar-mnt \
	--source.load=data/source-mnt.text \
	--hifst.lattice.store=$BASEDIR/trans.0002.fst \
	--stats.hifst.write=$BASEDIR/cykgrid.0002 \
	--stats.hifst.cykgrid.enable=yes \
	--stats.hifst.cykgrid.cellwidth=55 \
	--cykparser.hrmaxheight=20 --patternstoinstances.maxspan=20 \
	--logger.verbose  &> /dev/null

)
    if diff $BASEDIR/cykgrid.0002 $REFDIR/cykgrid.0002 ; then echo ; else echo 0; return ; fi
    rm -Rf tmp; mkdir -p tmp;
    cat $BASEDIR/trans.0002.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/trans.0002.test.fst;
    cat $REFDIR/trans.0002.fst | fstrmepsilon | fstdeterminize | fstminimize > tmp/trans.0002.ref.fst;
    if fstequivalent tmp/trans.0002.test.fst tmp/trans.0002.ref.fst; then echo ; else echo 0; return ; fi
# Ok!
    echo 1
}


### Testing hrmaxheight on a trivial grammar, with exceptions
test_0003_hifst_cykparser_maxspan(){

    $hifst \
	--grammar.load=data/rules/trivial.grammar3 \
	--source.load=data/source3.text \
	--cykparser.hrmaxheight=5  \
	--logger.verbose  > $BASEDIR/output.0003.1 2> /dev/null

    $hifst \
	--grammar.load=data/rules/trivial.grammar3 \
	--source.load=data/source3.text \
	--cykparser.hrmaxheight=3  \
	--logger.verbose  > $BASEDIR/output.0003.2 2> /dev/null

    $hifst \
	--grammar.load=data/rules/trivial.grammar3 \
	--source.load=data/source3.text \
	--cykparser.hrmaxheight=4  \
        --cykparser.ntexceptionsmaxspan=V \
	--logger.verbose  > $BASEDIR/output.0003.3 2> /dev/null


    $hifst \
	--grammar.load=data/rules/trivial.grammar3 \
	--source.load=data/source3.text \
	--cykparser.hrmaxheight=4  \
        --cykparser.ntexceptionsmaxspan=S,V \
	--logger.verbose  > $BASEDIR/output.0003.4 2> /dev/null


    if diff $BASEDIR/output.0003.1 $REFDIR/output.0003.1 ; then echo ; else echo 0; return ; fi
    if diff $BASEDIR/output.0003.2 $REFDIR/output.0003.2 ; then echo ; else echo 0; return ; fi
    if diff $BASEDIR/output.0003.3 $REFDIR/output.0003.3 ; then echo ; else echo 0; return ; fi
    if diff $BASEDIR/output.0003.4 $REFDIR/output.0003.4 ; then echo ; else echo 0; return ; fi

    echo 1;
}


### Testing a toy chop-grammar on a toy chop-sentence, chops marked by <sep>
### output changes slightly (now <sep>=999999992 appears in hyp), but scores should remain the same
### Using <sep> is better than using 0, enforces the existence of the rule in the lattice when aligning.
test_0004_hifst_chopgrammar(){

(  
    $hifst \
	--grammar.load=data/rules/trivial.grammar2b \
	--cykparser.ntexceptionsmaxspan=S,T,U,Q,R \
	--source.load=data/source2.text \
	--hifst.lattice.store=$BASEDIR/trans2.fst \
	--lm.load=data/lm/trivial.lm.gz  --hifst.prune=1 \
	--hifst.localprune.enable=yes --hifst.localprune.lm.load=data/lm/trivial.lm.gz --hifst.localprune.conditions=U,1,1,1,Q,1,1,1  \
	--stats.hifst.write=$BASEDIR/cykgrid2 --stats.hifst.cykgrid.enable=yes  &> /dev/null

)

     if diff $BASEDIR/cykgrid2 $REFDIR/cykgrid2 ; then echo ; else echo 0; return ; fi
     if fstequivalent $BASEDIR/trans2.fst $REFDIR/trans2.fst ; then echo ; else echo 0; return ; fi

#### now generate alilats
    $hifst \
	--grammar.load=data/rules/trivial.grammar2b \
	--cykparser.ntexceptionsmaxspan=S,T,U,Q,R \
	--source.load=data/source2.text \
        --referencefilter.load=$BASEDIR/trans2.fst \
        --referencefilter.prunereferenceshortestpath=1 \
	--hifst.lattice.store=$BASEDIR/alitrans2.fst \
	--hifst.alilatsmode=yes \
	--hifst.localprune.enable=yes --hifst.localprune.conditions=U,1,1,1,Q,1,1,1,T,1,1,1,R,1,1,1,S,1,1,1  \
    &> /dev/null

#### finally run alilats2splats

    $alilats2splats \
        --ruleflowerlattice.load=data/rules/trivial.grammar2b \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alitrans2.fst \
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/trans2.nbest\
        --sparseweightvectorlattice.storefeaturefile=$BASEDIR/trans2.features \
	--lm.load=data/lm/trivial.lm.gz \
	--lm.featureweights=1 &>/dev/null

     if diff $BASEDIR/trans2.nbest $REFDIR/trans2.nbest ; then echo ; else echo 0; return ; fi
    
# Ok!
    echo 1
}


### another weird grammar. Any "strange" rules
### worth testing, add them to trivial.grammar4.
test_0005_hifst_weird_grammar(){

( 
    $hifst \
	--grammar.load=data/rules/trivial.grammar4 \
	--source.load=data/source3.text \
	--hifst.lattice.store=$BASEDIR/trans.0005.fst \
	--lm.load=data/lm/trivial.lm.gz  \
	&> /dev/null
)

     if fstequivalent $BASEDIR/trans.0005.fst $REFDIR/trans.0005.fst ; then echo ; else echo 0; return ; fi
    
# Ok!
    echo 1
}

################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



