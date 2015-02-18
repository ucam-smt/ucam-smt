#!/bin/bash

source runtests.sh
hifst=$CAM_SMT_DIR/bin/hifst.${TGTBINMK}.bin
alilats2splats=$CAM_SMT_DIR/bin/alilats2splats.${TGTBINMK}.bin
grammar=data/rules/trivial.grammar 
tstidx=data/source.text
languagemodel=data/lm/trivial.lm.gz
languagemodeltrie=data/lm/trivial.lm.trie.mmap
weaklanguagemodel=data/lm/trivial.lm.gz
wlanguagemodel=data/lm/trivial.lm.words.gz
wweaklanguagemodel=data/lm/trivial.lm.words.gz
nplmmodel=data/lm/inferno.nnlm
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

test_0002_alilats2splats_found(){
    if [ ! -f $alilats2splats ]; then echo 0 ; return; fi
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


test_0010_translate() {

( #set -x
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats/?.fst.gz  \
        --lm.load=$languagemodel \
	--grammar.storentorder=nttable \
        --hifst.prune=9  &>/dev/null

)
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/lats/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/lats/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1
}


test_0011_align() {

(  #set -x
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --referencefilter.load=$BASEDIR/lats/?.fst.gz \
        --referencefilter.prunereferenceshortestpath=2 \
        --hifst.lattice.store=$BASEDIR/alilats/?.fst.gz  \
        --hifst.localprune.enable=yes  --hifst.localprune.conditions=X,1,1,9,M,1,1,9,V,1,1,9 \
        --hifst.prune=9  \
	--hifst.alilatsmode=yes    &>/dev/null
)

 
    seqrange=`echo $range | sed -e 's:\:: :g'`

    for k in `seq $seqrange`; do 

	mkdir -p tmp; 
### Note: fstproject - openfst 1.3.1  fails
	zcat $BASEDIR/alilats/$k.fst.gz | fstproject | fstrmepsilon | fstdeterminize | fstminimize | fstrmepsilon > tmp/$k.test.fst; 
	zcat $REFDIR/alilats/$k.fst.gz | fstproject | fstrmepsilon | fstdeterminize | fstminimize | fstrmepsilon > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	


#	if [ "`zcat $BASEDIR/alilats/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi; 
#	if [ "`zcat $BASEDIR/alilats/$k.fst.gz | fstprint | md5sum`" != "`zcat $REFDIR/alilats/$k.fst.gz | fstprint | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}


test_0012_extractFeatures() {

( # set -x
    $alilats2splats \
	--range=$range \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
	--sparseweightvectorlattice.store=$BASEDIR/vwlats/?.fst.gz\
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest/?.nbest.gz\
        --sparseweightvectorlattice.storefeaturefile=$BASEDIR/fea/?.features.gz \
	--lm.load=$languagemodel \
	--lm.featureweights=1 &>/dev/null

)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nbest/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi; 
	if [ "`zcat $BASEDIR/fea/$k.features.gz | md5sum`" != "`zcat $REFDIR/fea/$k.features.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}


test_0013_translate_pdt() {

     $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
	--hifst.lattice.store=$BASEDIR/pdt-lats/?.fst.gz  \
        --lm.load=$languagemodel \
	--hifst.replacefstbyarc.numstates=0 \
	--hifst.usepdt=yes --hifst.rtnopt=no  &>/dev/null    
#         --hifst.prune=9    ### Would be a very good idea to actually do pruned expansion (and a use a weak language model)

    rm -Rf tmp; mkdir -p tmp;
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
	if [ "`zcat $BASEDIR/pdt-lats/$k.fst.gz | fstinfo`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/pdt-lats/$k.fst.gz  | fstprint | sed -e s:,.*$::g | fstcompile \
       	    | pdtexpand --pdt_parentheses=TESTFILES/test-hifst+alilats2splats/pdt-lats/$k.fst.gz.parens  \
	    | fstrmepsilon | fstdeterminize | fstminimize > tmp/$k.test.fst;

	zcat $REFDIR/lats/$k.fst.gz  | fstprint | sed -e s:,.*$::g | fstcompile > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;	
    done

###Success                              
    echo 1
}



test_0014_translate_pdt2() {
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.usepdt=yes \
        --hifst.lattice.store=$BASEDIR/pdt-lats2/?.fst.gz  \
	--lm.featureweights=1 --lm.load=$languagemodel --lm.wps=0\
        --hifst.localprune.enable=yes --hifst.localprune.conditions=S,-1,1,9 --hifst.localprune.lm.featureweights=1 --hifst.localprune.lm.load=$weaklanguagemodel --hifst.localprune.lm.wps=-2.30 \
        --hifst.prune=9 \
        2>&1    &>/dev/null

# Notes: 
# - PDT representation is internal. After language model rescoring it generates FSAs
# - Cell pruning on the topmost cell = admissible pruning
# - This option activates RTN optimizations provided by OpenFST. --hifst.nortnopt 
#   However, it is not always beneficial.


    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/pdt-lats2/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/pdt-lats2/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1

}



test_0015_translate_2feat() {

( 
    $hifst \
        --grammar.load=$grammar-2f --grammar.featureweights=1,1 \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats-2f/?.fst  \
        --lm.load=$languagemodel \
        --hifst.prune=9    &>/dev/null

)

## Compare to REFDIR/lats, should be the same

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/lats-2f/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst $BASEDIR/lats-2f/$k.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1
}




test_0016_translate_nthreads() {

( # set -x
    $hifst --nthreads=4 \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats-nth/?.fst.gz  \
        --lm.load=$languagemodel \
        --hifst.prune=9    &>/dev/null

)
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/lats/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/lats-nth/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1
}


test_0017_align_nthreads() {

(  # set -x
    $hifst --nthreads=4 \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --referencefilter.load=$BASEDIR/lats-nth/?.fst.gz \
        --referencefilter.prunereferenceshortestpath=2 \
        --hifst.lattice.store=$BASEDIR/alilats-nth/?.fst.gz  \
        --hifst.localprune.enable=yes  --hifst.localprune.conditions=X,1,1,9,M,1,1,9,V,1,1,9 \
        --hifst.prune=9  \
	--hifst.alilatsmode=yes    &>/dev/null

#        --lm.scales=1 --lm.load=$languagemodel \

)

 
    seqrange=`echo $range | sed -e 's:\:: :g'`

    for k in `seq $seqrange`; do 

	mkdir -p tmp; 
### Note: fstproject - openfst 1.3.1  fails
	zcat $BASEDIR/alilats-nth/$k.fst.gz | fstproject | fstrmepsilon | fstdeterminize | fstminimize | fstrmepsilon > tmp/$k.test.fst; 
	zcat $REFDIR/alilats/$k.fst.gz | fstproject | fstrmepsilon | fstdeterminize | fstminimize | fstrmepsilon > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	

    done

###Success
    echo 1
}


test_0018_extractFeatures_nthreads() {
    $alilats2splats --nthreads=4 \
	--range=$range \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats-nth/?.fst.gz \
	--sparseweightvectorlattice.store=$BASEDIR/vwlats-nth/?.fst.gz\
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest-nth/?.nbest.gz\
        --sparseweightvectorlattice.storefeaturefile=$BASEDIR/fea-nth/?.features.gz \
	--lm.load=$languagemodel \
	--lm.featureweights=1 &>/dev/null
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nbest-nth/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi; 
	if [ "`zcat $BASEDIR/fea-nth/$k.features.gz | md5sum`" != "`zcat $REFDIR/fea/$k.features.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}


# Same test as 0014, but now using word lms.
test_0019_translate_pdt2_use_wordlm() {
( #set -x
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.usepdt=yes \
        --hifst.lattice.store=$BASEDIR/pdt-lats-wlm/?.fst.gz  \
	--lm.featureweights=1 --lm.load=$languagemodel --lm.wps=0\
        --hifst.localprune.enable=yes --hifst.localprune.conditions=S,-1,1,9 --hifst.localprune.lm.featureweights=1 --hifst.localprune.lm.load=$weaklanguagemodel --hifst.localprune.lm.wps=-2.30 \
        --hifst.prune=9 \
        2>&1    &> /dev/null
)


    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/pdt-lats-wlm/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/pdt-lats-wlm/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1

}


# test alilats2splats with word lm
test_0020_wlm_extractFeatures() {

( #set -x
    $alilats2splats \
	--range=$range \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
	--sparseweightvectorlattice.store=$BASEDIR/vwlats-wlm/?.fst.gz\
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest-wlm/?.nbest.gz\
        --sparseweightvectorlattice.storefeaturefile=$BASEDIR/fea-wlm/?.features.gz \
	--lm.load=$wlanguagemodel --lm.wordmap=data/sunmap\
	--lm.featureweights=1 &>/dev/null

)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nbest-wlm/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi; 
	if [ "`zcat $BASEDIR/fea-wlm/$k.features.gz | md5sum`" != "`zcat $REFDIR/fea/$k.features.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}


test_0021_extractNbestWordmap() {

( # set -x
    $alilats2splats \
	--range=$range \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest-wm/?.nbest.gz\
        --sparseweightvectorlattice.wordmap=data/sunmap\
	--lm.load=$languagemodel \
	--lm.featureweights=1 &>/dev/null

)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nbest-wm/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest-wm/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}


test_0022_extractNbestWordmapMultiThreading() {

( #set -x
    $alilats2splats \
	--range=$range \
	--nthreads=2 \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest-wm-mult/?.nbest.gz\
        --sparseweightvectorlattice.wordmap=data/sunmap\
	--lm.load=$languagemodel \
	--lm.featureweights=1 &>/dev/null

)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nbest-wm-mult/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest-wm/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}


### Use program option featureweights, should yield same result as 0015.
### featureweights contains 1 for the language model and the next two for the grammar.
### should ignore any values defined in grammar.featureweights and or lm.featureweights,
### e.g. see below
test_0023_translate_2feat() {

( 
    $hifst \
	--featureweights=1,1,1 --grammar.featureweights=0.5 --lm.featureweights=1.5,9,10,11,12 \
        --grammar.load=$grammar-2f \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats-2f/?.0023.fst  \
        --lm.load=$languagemodel \
        --hifst.prune=9    &>/dev/null

)

## Compare to REFDIR/lats, should be the same

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`cat $BASEDIR/lats-2f/$k.fst | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst $BASEDIR/lats-2f/$k.0023.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1
}

test_0024_extractFeaturesFeatureweights() {

( # set -x
    $alilats2splats \
	--range=$range \
	--featureweights=1,1 \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
	--sparseweightvectorlattice.store=$BASEDIR/vwlats.0024/?.fst.gz\
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest.0024/?.nbest.gz\
        --sparseweightvectorlattice.storefeaturefile=$BASEDIR/fea.0024/?.features.gz \
	--lm.load=$languagemodel &>/dev/null

)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nbest.0024/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi; 
	if [ "`zcat $BASEDIR/fea.0024/$k.features.gz | md5sum`" != "`zcat $REFDIR/fea/$k.features.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}

test_0025_extractFeaturesStripHifstEpsilons() {

( # set -x
    $alilats2splats \
	--range=3 \
	--featureweights=1,1 \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest.0025/?.nbest.gz\
        --sparseweightvectorlattice.stripspecialepsilonlabels=yes \
	--lm.load=$languagemodel &>/dev/null

)

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in 3; do
	if [ "`zcat $BASEDIR/nbest.0025/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest/$k.nbest.stripped.gz | md5sum`" ] ; then echo 0; return ; fi; 
    done

###Success
    echo 1
}

test_0026_translate_nnlm() {

    if [ -z $NPLM_LIB ]; then
	echo 2
	return
    fi
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/nnlm-lats/?.fst.gz  \
        --lm.load=data/lm/inferno.nnlm \
        --hifst.prune=9  &>/dev/null
    rm -Rf tmp
    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/nnlm-lats/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;  
	mkdir -p tmp; zcat $BASEDIR/nnlm-lats/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/nnlm-lats/$k.fst.gz > tmp/$k.ref.fst;	
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ; 	
    done

###Success
    echo 1
}

test_0027_translate_two_languagemodels() {

# Same language model used twice, 0.5 feature weight for each, should get 0010 results.
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats.2lm/?.fst.gz  \
        --lm.load=$languagemodel,$languagemodel --lm.featureweights=0.5,0.5 --lm.wps=0,0 \
        --hifst.prune=9  &>/dev/null

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
	if [ "`zcat $BASEDIR/lats.2lm/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/lats.2lm/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;
    done

# Repeat exercise, now using the same second language model but in trie format, still same results
    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats.2lmq/?.fst.gz  \
        --lm.load=$languagemodel,$languagemodeltrie --lm.featureweights=0.5,0.5 --lm.wps=0,0 \
        --hifst.prune=9  &>/dev/null

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
	if [ "`zcat $BASEDIR/lats.2lmq/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/lats.2lmq/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;
    done
###Success
    echo 1
}

test_0028_translate_arpa+trie+nplm_languagemodels() {
    if [ -z $NPLM_LIB ]; then
	echo 2
	return
    fi

    $hifst \
        --grammar.load=$grammar \
        --source.load=$tstidx  \
        --hifst.lattice.store=$BASEDIR/lats.3lm/?.fst.gz  \
        --lm.load=$languagemodel,$languagemodeltrie,$nplmmodel --lm.featureweights=0.5,0.5,1 --lm.wps=0,0,0 \
        --hifst.prune=9  &>/dev/null

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do 
	if [ "`zcat $BASEDIR/lats.3lm/$k.fst.gz | fstprint | md5sum`" == "" ] ; then echo 0; return ; fi;
	mkdir -p tmp; zcat $BASEDIR/lats.3lm/$k.fst.gz > tmp/$k.test.fst; zcat $REFDIR/lats.3lm/$k.fst.gz > tmp/$k.ref.fst;
	if fstequivalent tmp/$k.ref.fst tmp/$k.test.fst; then echo -e ""; else echo 0; return; fi ;
    done
###Success
    echo 1
}

#### Repeat alilats2splats with the three models
test_0029_extractFeatures_arpa+trie+nplm() {
    if [ -z $NPLM_LIB ]; then
	echo 2
	return
    fi
    $alilats2splats \
	--range=$range \
	--ruleflowerlattice.load=$grammar \
	--ruleflowerlattice.filterbyalilats \
	--sparseweightvectorlattice.loadalilats=$BASEDIR/alilats/?.fst.gz \
	--sparseweightvectorlattice.store=$BASEDIR/vwlats.0029/?.fst.gz\
        --sparseweightvectorlattice.storenbestfile=$BASEDIR/nbest.0029/?.nbest.gz\
        --sparseweightvectorlattice.storefeaturefile=$BASEDIR/fea.0029/?.features.gz \
	--lm.load=$languagemodel,$languagemodeltrie,$nplmmodel \
	--lm.featureweights=0.5,0.5,1 --lm.wps=0,0,0 &>/dev/null

    seqrange=`echo $range | sed -e 's:\:: :g'`
    for k in `seq $seqrange`; do
	if [ "`zcat $BASEDIR/nbest.0029/$k.nbest.gz | md5sum`" != "`zcat $REFDIR/nbest.0029/$k.nbest.gz | md5sum`" ] ; then echo 0; return ; fi;
	if [ "`zcat $BASEDIR/fea.0029/$k.features.gz | md5sum`" != "`zcat $REFDIR/fea.0029/$k.features.gz | md5sum`" ] ; then echo 0; return ; fi;
    done

###Success
    echo 1
}





################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



