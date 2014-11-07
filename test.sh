#!/bin/bash

if [ -z "$TGTBINMK" ]; then

    cat <<EOF

Environment variable TGTBINMK not defined. 
Please use one of the following options:

std          : std compilation
O2           : compilation with O2 optimization
O3           : compilation with O3 optimization
sta          : static compilation
sta.O2       : static compilation with O2 optimization
sta.O3       : static compilation with O3 optimizaiton
sta.gdb      : static compilation with extra flags for gdb symbols
debug0       : compilation with extra flags for gdb symbols, binary only prints intermediate lattice files
debug        : compilation with extra flags for gdb symbols, binary prints intermediate lattice files, also verbosity increased with dbg messages
sta.debug0   : static compilation with extra flags for gdb symbols, binary only prints intermediate lattice files
sta.debug    : static compilation with extra flags for gdb symbols, binary prints intermediate lattice files, also verbosity increased with dbg messages
pro          : compile for profiling
pro.O2       : compile for profiling, including O2 optimizations

EOF

    exit

fi 

export TGTBINMKTEST=`echo $TGTBINMK | sed -e 's:\..*$::g'`
if [ "$TGTBINMKTEST" != "sta" ]; then
    TGTBINMKTEST="std"; # unit tests always compile on debug mode
fi
echo "Compiling with TGTBINMK=$TGTBINMK"


LD_LIBRARY_PATHORIG=$LD_LIBRARY_PATH
PATHORIG=$PATH
LOGFILE=$(basename $0 .sh).$TGTBINMK.log

. Makefile.inc
. scripts/checkenvars.sh
checkVariables

export LD_LIBRARY_PATH=$PWD/bin/:$OPENFST_LIB:$GTEST_LIB:$BOOST_LIB:$LD_LIBRARY_PATH
PATH=$OPENFST_BIN:$PATH

echo "Tests started on $(date) on $(hostname)" >> $LOGFILE

### All tests should pass
echo "Running Unit Tests... bin/main.gtest.$TGTBINMKTEST" | tee -a $LOGFILE
( 
    if [ -e bin/main.gtest.$TGTBINMKTEST ]; then 
	if bin/main.gtest.$TGTBINMKTEST ; then
	    echo "====================="
	    echo "====================="
	    echo "====================="
	    echo "Unit tests --- SUCCESS"
	else
	    echo "====================="
	    echo "====================="
	    echo "====================="
	    echo "Unit tests --- FAILED!"
	fi
    else 
	echo "====================="
	echo "====================="
	echo "====================="
	echo "Unit tests --- Missing???"; 
    fi 
) 2>&1 | tee $LOGFILE.unit-tests

(cd java/ruleXtract; sbt test) 2>&1 | tee -a $LOGFILE.java-unit-tests

echo "Running Regression Tests..." 2>&1 | tee -a $LOGFILE
( cd scripts/tests; bash tests.sh  ) 2>&1 | tee $LOGFILE.reg-tests

rm -Rf fsts

echo "Tests finished on $(date) on $(hostname). RESULTS:" | tee -a $LOGFILE
tail -v -n 7 $LOGFILE.unit-tests | tee -a $LOGFILE
tail -v -n 4 $LOGFILE.reg-tests | tee -a $LOGFILE
tail -v -n 4 $LOGFILE.java-unit-tests | tee -a $LOGFILE
echo "Done!" 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATHORIG
export PATH=$PATHORIG
