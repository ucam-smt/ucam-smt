#!/bin/bash

GTESTVERSION=1.7.0
BOOSTVERSION=1_53_0
OPENFSTVERSION=1.3.4 

installExternals() {
    (
	if [ ! -e gtest-$GTESTVERSION ]; then 
	    wget https://googletest.googlecode.com/files/gtest-$GTESTVERSION.zip; 
	    unzip gtest-$GTESTVERSION.zip ; 
	    cd gtest-$GTESTVERSION; 
	    ./configure --enable-static --prefix=$PWD/INSTALL_DIR; 
	    make 
# NOT supported for gtest-1.6.0 , gtest-1.7.0
#	    make install
	    mkdir -p INSTALL_DIR/include INSTALL_DIR/lib
	    cp -a include/* INSTALL_DIR/include
	    cp -a lib/.libs/* INSTALL_DIR/lib
	fi
	)

    (
	if [ ! -e openfst-$OPENFSTVERSION ]; then 
	    wget http://www.openfst.org/twiki/pub/FST/FstDownload/openfst-$OPENFSTVERSION.tar.gz   
	    tar zxvf openfst-$OPENFSTVERSION.tar.gz
	    cd openfst-$OPENFSTVERSION
	    ./configure --prefix=$PWD/INSTALL_DIR \
		--enable-bin \
		--enable-compact-fsts \
		--enable-const-fsts \
		--enable-far \
		--enable-lookahead-fsts \
		--enable-pdt \
		--enable-static \
		--enable-ngram-fsts
	    make
	    make install
	fi
	)
    
    
    (
	if [ ! -e boost_$BOOSTVERSION ]; then 
	    bv=`echo $BOOSTVERSION | tr '_' '.'`
	    wget http://sourceforge.net/projects/boost/files/boost/$bv/boost_$BOOSTVERSION.tar.gz/download 
	    mv download boost_$BOOSTVERSION.tar.gz
	    
	    tar zxvf boost_$BOOSTVERSION.tar.gz
	    
	    cd boost_$BOOSTVERSION
	    ./bootstrap.sh --prefix=$PWD/INSTALL_DIR/
	    ./b2 -sNO_BZIP2=1 install link=shared
	    ./b2 -sNO_BZIP2=1 install link=static
	fi
	)

}


## Installing open source release 
prepareMakefileInc() {
    export TOPDIR;
    pushd ../;TOPDIR=$PWD; popd
    export GTEST_DIR=$TOPDIR/externals/gtest-$GTESTVERSION
    echo "### Modify these env variables to point to your boost/openfst installation,
export BOOST_INCLUDE=$TOPDIR/externals/boost_$BOOSTVERSION/INSTALL_DIR/include
export BOOST_LIB=$TOPDIR/externals/boost_$BOOSTVERSION/INSTALL_DIR/lib

export OPENFSTVERSION=$OPENFSTVERSION
export OPENFST_INCLUDE=$TOPDIR/externals/openfst-${OPENFSTVERSION}/INSTALL_DIR/include
export OPENFST_LIB=$TOPDIR/externals/openfst-${OPENFSTVERSION}/INSTALL_DIR/lib
export OPENFST_BIN=$TOPDIR/externals/openfst-${OPENFSTVERSION}/INSTALL_DIR/bin

## For google tests:
export GTEST_DIR=$GTEST_DIR;
export GTEST_INCLUDE=${GTEST_DIR}/include
export GTEST_LIB=${GTEST_DIR}/INSTALL_DIR/lib

# Lattice mert (latmert) can compile with gperftools and Unwind. You need to install
# these libraries and then modify Makefile.inc so that these three variables point
# to the right places:
# export GPERFTOOLS_INCLUDE=$TOPDIR/externals/gperftools-2.0/INSTALL_DIR/include
# export GPERFTOOLS_LIB=$TOPDIR/externals/gperftools-2.0/INSTALL_DIR/lib
# export UNWIND_LIB=$TOPDIR/externals/libunwind-1.0.1-install/lib


### Please write in here the absolute path to HiFST:
export TOPDIR=$TOPDIR

### Default KENLM package -- bundled with HiFST package.
export KENLM_DIR=${TOPDIR}/externals/kenlm/


#### Number of cpus to be used for compilation. 
export NUMPROC=1

#### Compilation type. This has been decided the first time you run build-test.sh
#### The recommended default value is O2, but if you do
#### > export TGTBINMK=...  (see build-test.sh)
#### __before__ running build-test.sh, this configuration will get written here.
export TGTBINMK=${TGTBINMK}

" > Makefile.inc 
	    
}

#installExternals
#prepareMakefileInc