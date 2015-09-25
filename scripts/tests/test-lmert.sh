
#!/bin/bash

source runtests.sh
lmert=$CAM_SMT_DIR/bin/lmert.${TGTBINMK}.bin
paramsfile=data/lmert/params.0
reffile=data/lmert/refs
veclatsdir=data/lmert/veclats
### Up to 99 sentences:
range=1:20
# range=1:99

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR;

export PATH=$OPENFST_BIN:$PATH
################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

test_0001_lmert_found(){
    if [ ! -f $lmert ]; then echo 0 ; return;  fi
    echo 1
}

test_0002_lmert_run() {
    mkdir -p $BASEDIR

    $lmert --logger.verbose \
        --input=data/lmert/VECFEA/?.fst.gz \
	--initial_params=file://$paramsfile \
	--int_refs=$reffile \
	--range=$range  \
	--min_gamma=1.0 \
	--random_seed=17 \
	--nthreads=24 \
	--write_params=$BASEDIR/newparams &>/dev/null

	if [ "$range" == "1:99" ]; then 
	   if diff $BASEDIR/newparams $REFDIR/newparams.1-99 ; then echo ; else echo 0; return ; fi
	elif [ "$range" == "1:20" ]; then
	    # instead of comparing to: $REFDIR/newparams.1-20
	   if diff $BASEDIR/newparams $REFDIR/newparams.1-20.boost1.59 ; then echo ; else echo 0; return ; fi
	fi

###Success
#    echo 1
###Flagged
# Changed references when upgrading boost 1.53 to 1.59
# Untested working hypothesis: might be due to changes related to uniform_on_sphere.hpp
# lmert run/log looks good, but lets wait to mark this test as success until we have
# confirmation that the tool is tuning parameters correctly on real tasks.
# Note that latmert regression tests ran ok.
    echo 3
}



################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES

runtests



