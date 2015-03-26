#!/bin/bash

source runtests.sh

DEMO_FILES_DIR="$CAM_SMT_DIR/../demo-files"
export PATH=$TOPDIR/bin:$PATH

BASEDIR=TESTFILES/`basename $0 | sed -e 's:.sh::g'`
REFDIR=REFFILES/`basename $0 | sed -e 's:.sh::g'`
rm -fR $BASEDIR; mkdir -p $BASEDIR;

################### Criteria
################### - Each bash test file is considered an independent test
################### - Each bash test file contains a series of test functions test_NUMBER_NAME. They return 1 if success or 0 if failure.
################### - These functions do not necessarily have to be independent from each other. The NUMBER in the test should be unique to guarantee they get executed in the proper order

### Ensure that all current demo files exist.
### If new files are added, please add them to $REFDIR/testfiles!
test_0001_demofiles_found() {
    # skip because demo-files directory is not available
    if [ ! -d $DEMO_FILES_DIR ] ; then echo 2; return; fi 
    R=`cat $REFDIR/testfiles | while read file; do
	if [ ! -e $file ]; then echo FAIL; return;fi
    done`
    if [ "$R" == "FAIL" ]; then echo 0; else echo 1; fi;
}


grep_and_run_test() {
    tf=$1;
(
    pushd $DEMO_FILES_DIR &> /dev/null;
    grep "::>" Docs.dox/$tf | sed -e 's:^  *\:\:> *::g;s:&>.*$::g;s:CF.baseline.server:CF.baseline.server \&>dev.null \&:g' >  test.bash  
    bash test.bash 2>&1 | sed -e 's:^.*.INF:INF:g;s:^.*.WRN:WRN:g;s:^real.*s$::g;s:^user.*s$::g;s:^sys.*s$::g;s:^INF.* starts!:START:g;s:^INF.* ends!:END:g;' # take out the date stamps
    rm test.bash
    popd &> /dev/null
) > $BASEDIR/$tf.logs

}

### Capture commands from Tutorial.010.basic.md we want to test with a grep command
### and test them. Commands that we test are flagged with "::>", in contrast to other
### other commands, which only have ">".
test_0002_demofiles_basic_script(){
    # skip because demo-files directory is not available
    if [ ! -d $DEMO_FILES_DIR ] ; then echo 2; return; fi 
    (cd $DEMO_FILES_DIR/wmaps; for file in `ls *.gz`; do nfile=`echo $file | sed -e 's:.gz$::g'`; gunzip -c $file > $nfile; done ) &>/dev/null
    testfile=Tutorial.010.basic.md
    grep_and_run_test $testfile
    diff $BASEDIR/$testfile.logs $REFDIR/$testfile.logs
    if [ "$?" == "1" ]; then echo 0; return; fi
    echo 1;
}

test_0003_demofiles_pda_script(){
    # skip because demo-files directory is not available
    if [ ! -d $DEMO_FILES_DIR ] ; then echo 2; return; fi 
    testfile=Tutorial.050.pda.md
    grep_and_run_test $testfile
    diff $BASEDIR/$testfile.logs $REFDIR/$testfile.logs
    if [ "$?" == "1" ]; then echo 0; return; fi
    echo 1;
}

################### STEP 2
################### RUN ALL TESTS AND PRINT MESSAGES
#test_0002_demofiles_basic_script
runtests
