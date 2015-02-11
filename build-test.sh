#!/bin/bash

if [ -z "$TGTBINMK" ]; then

    export TGTBINMK=O2
    cat <<EOF

Environment variable TGTBINMK not defined. Using 

  export TGTBINMK=$TGTBINMK

For a different target re-run this script using one of the following options

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



fi 

export TGTBINMKTEST=`echo $TGTBINMK | sed -e 's:\..*$::g'`

echo "Compiling with TGTBINMK=$TGTBINMK"

LOGFILE=$(basename $0 .sh).$TGTBINMK.log


if [ ! -e Makefile.inc ]; then
    echo "Makefile.inc does not exist. Do you want to try to download and install all necessary libraries? (yes/no)"
    read reply
    if [ "$reply" == "yes" ]; then
	echo -e "\n\nOk.Here goes nothing!"
	( cd externals
	    . get-build-libraries.sh
	    installExternals
	    prepareMakefileInc
	    cp Makefile.inc ..
	)
    else
	echo "You need to create Makefile.inc first. Copy from Makefile.inc.TEMPLATE and set variables to point to your libraries and headers."
	exit
    fi
fi


. Makefile.inc
. scripts/checkenvars.sh
checkVariables


openfstversion=`echo $OPENFSTVERSION | sed -e 's:\.:00:g'`

# Generate a constant to let compiler know the openfst version
echo -e "#ifndef OPENFSTVERSION\n#define OPENFSTVERSION $openfstversion\n#endif" > cpp/include/openfstversion.hpp

echo "Compilation started on $(date) on $(hostname)" > $LOGFILE

mkdir -p bin; 
rm -f bin/*$TGTBINMK* bin/main.gtest.$TGTBINMKTEST
(cd externals; bash prepare.bash) 2>&1 | tee -a $LOGFILE 
if [ ! -e bin/libkenlm.a ]; then
    echo "ERROR: KenLM not compiled (externals/prepare.bash). Exiting..."
    exit
fi

(cd cpp; make clean; make) 2>&1 | tee -a $LOGFILE

(cd java/ruleXtract; sbt package) 2>&1 | tee -a $LOGFILE

echo "Compilation ended on $(date) on $(hostname)" >> $LOGFILE

### Finally, run tests...
. test.sh
