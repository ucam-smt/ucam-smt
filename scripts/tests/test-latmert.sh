
#!/bin/bash

source runtests.sh
latmert=$CAM_SMT_DIR/bin/latmert.${TGTBINMK}.bin
paramsfile=data/latmert/params.0
reffile=data/latmert/refs
wmap=data/latmert/wordmap
veclatsdir=data/latmert/veclats
range=1:10

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

export PATH=$OPENFST_BIN:$PATH
################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_latmert_found(){
    if [ ! -f $latmert ]; then echo 0 ; return;  fi
    echo 1
}

test_0002_latmert_run() {
    mkdir -p $BASEDIR
#TODO: replace/add serious tests.
    $latmert \
	--seed=1366599232 \
	--search=random \
	--random_axes \
	--random_directions=1 \
	--threads=24 \
	--cache_lattices \
	--algorithm=lmert \
	--idxlimits=$range \
	--lambda=file:$paramsfile \
	--direction=axes \
	--print_precision=6 \
	--write_parameters=$BASEDIR/newparams \
	--lats=$veclatsdir/%idx%.fst.gz \
	$reffile &>/dev/null

	if diff $BASEDIR/newparams $REFDIR/newparams ; then echo ; else echo 0; return ; fi

###Success
    echo 1
}



################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



