#!/bin/bash

export CAM_SMT_DIR=../../
. $CAM_SMT_DIR/Makefile.inc

if [ -z "$TGTBINMK" ]; then

    cat <<EOF

Environment variable TGTBINMK not defined. Use

  export TGTBINMK=value

with value being one of the following

std          : std compilation
O2           : compilation with O2 optimization
O3           : compilation with O3 optimization
sta            : static compilation
sta.O2         : static compilation with O2 optimization
sta.O3         : static compilation with O3 optimizaiton
sta.gdb        : static compilation with extra flags for gdb symbols
debug0       : compilation with extra flags for gdb symbols, binary only prints intermediate lattice files
debug        : compilation with extra flags for gdb symbols, binary prints intermediate lattice files, also verbosity increased with dbg messages
sta.debug0     : static compilation with extra flags for gdb symbols, binary only prints intermediate lattice files
sta.debug      : static compilation with extra flags for gdb symbols, binary prints intermediate lattice files, also verbosity increased with dbg messages
pro          : Compile for profiling
pro.O2       : Compile for profiling, including O2 optimizations

EOF
  
    exit 1
fi






export LD_LIBRARY_PATH=$CAM_SMT_DIR/bin/:$OPENFST_LIB:$BOOST_LIB:$LD_LIBRARY_PATH
export PATH=$OPENFST_BIN:$PATH
if [ ! -z $NPLM_LIB ]; then LD_LIBRARY_PATH+=:$NPLM_LIB; fi

runtests() {

    echo "RUNNING TESTS IN `basename $0`"
    echo "============================================================"
    
    total=0
    passed=0
    failed=0
    skip=0
    for mytest in `typeset -F | awk '{print $3}'| grep "^test_" `; do 
	let total=total+1
	if [[ `$mytest | tail -1 ` -eq 1 ]]; then 
	printf "OK!   ========== %-50s\n" $mytest
	let passed=passed+1
	elif [[ `$mytest | tail -1 ` -eq 2 ]]; then 
	    printf "SKIP! ========== %-50s\n" $mytest
	    let skip=skip+1
	else
	    printf "FAIL! ========== %-50s\n" $mytest
	    let failed=failed+1
	fi
    done 
    
    echo "============================================================"
    printf " PASSED: %5d tests (%20s)\n" $passed `basename $0`
    printf " FAILED: %5d tests (%20s) \n" $failed `basename $0`
    printf "SKIPPED: %5d tests (%20s) \n" $skip `basename $0`
    printf "  TOTAL: %5d tests (%20s) \n" $total `basename $0`
}
