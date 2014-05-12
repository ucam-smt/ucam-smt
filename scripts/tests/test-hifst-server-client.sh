#!/bin/bash

source runtests.sh

hifst=$CAM_SMT_DIR/bin/hifst.${TGTBINMK}.bin
hifstclient=$CAM_SMT_DIR/bin/hifst-client.${TGTBINMK}.bin
alilats2splats=$CAM_SMT_DIR/bin/alilats2splats.${TGTBINMK}.bin
grammar=data/rules/trivial.grammar 
tstidx=data/source.text
test=data/source-wmap.txt
smap=data/smap
sunmap=data/sunmap
languagemodel=data/lm/trivial.lm.gz
weaklanguagemodel=data/lm/trivial.lm.gz
range=1:4

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

export PATH=$OPENFST_BIN:$PATH
################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order


test_0001_serverclientfound() {
    if [ ! -f $hifst ]; then echo 0 ; return; fi
    if [ ! -f $hifstclient ]; then echo 0 ; return; fi
    echo 1
}

test_0002_translate-server-client() {
    mkdir -p $BASEDIR
    $hifst --server.enable=yes  --server.port=1205 --logger.verbose \
	--grammar.load=$grammar \
	--lm.load=$languagemodel \
	&>$BASEDIR/server.log &
    pid=$!	
    
    echo "hifst server pid=$pid"
    sleep 1
    $hifstclient \
	--host=localhost --port=1205 \
	--source.load=$tstidx \
	--target.store=$BASEDIR/translation.txt &> /dev/null
        
    kill -9 $pid  
    wait $pid 2>/dev/null    


    if diff $BASEDIR/translation.txt $REFDIR/translation.txt ; then echo 1; return ; fi;
    echo 0; 
}


### Note: tokenization/detokenization not implemented.
### Once we have it in place, 
test_0003_translate-server-client-wmaps() {

    $hifst --server.enable=yes  --server.port=1205 --logger.verbose \
	--grammar.load=$grammar \
	--lm.load=$languagemodel \
	--prepro.tokenize.enable=no --prepro.addsentencemarkers --prepro.wordmap.load=$smap \
	--postpro.detokenize.enable=no --postpro.capitalizefirstword.enable=yes --postpro.wordmap.load=$sunmap \
	&>$BASEDIR/server-wmap.log &

    pid=$!	

    
    echo hifst server pid=$pid
    sleep 1
    $hifstclient \
	--host=localhost --port=1205 \
	--source.load=$test \
	--target.store=$BASEDIR/translation-wmap.txt &> /dev/null
        
    kill -9 $pid  
    wait $pid 2>/dev/null    

    if diff $BASEDIR/translation-wmap.txt $REFDIR/translation-wmap.txt ; then echo 1; return ; fi;
    echo 0; 

}


test_0004_translate-wmaps() {

( 
    $hifst --logger.verbose \
	--source.load=$test \
	--grammar.load=$grammar \
	--lm.load=$languagemodel \
	--prepro.tokenize.enable=yes --prepro.addsentencemarkers --prepro.wordmap.load=$smap \
	--postpro.detokenize.enable=yes --postpro.capitalizefirstword.enable=yes --postpro.wordmap.load=$sunmap \
	--target.store=$BASEDIR/translation-wmap2.txt \
	 &> /dev/null
)

    if diff $BASEDIR/translation-wmap2.txt $REFDIR/translation-wmap.txt ; then echo 1; return ; fi;
    echo 0; 





}





################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



