
#!/bin/bash

source runtests.sh
lmert=$CAM_SMT_DIR/bin/lmert.${TGTBINMK}.bin
paramsfile=data/lmert/params.0
reffile=data/lmert/refs
veclatsdir=data/lmert/veclats
range=1:99

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

export PATH=$OPENFST_BIN:$PATH
################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_latmert_found(){
    if [ ! -f $lmert ]; then echo 0 ; return;  fi
    echo 1
}

test_0002_latmert_run() {
    mkdir -p $BASEDIR
    $lmert \
        --input=data/lmert/VECFEA/?.fst.gz \
	--initial_params=file:$paramsfile \
	--semiring=tuplearc \
	--refs=$reffile \
	--range=$range \
	--min_gamma=1.0 \
	--random_seed=17 \
	--num_threads=24 \
	--write_params=$BASEDIR/newparams &>/dev/null

	if diff $BASEDIR/newparams $REFDIR/newparams ; then echo ; else echo 0; return ; fi

###Success
    echo 1
}



################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



