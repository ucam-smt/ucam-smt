#!/bin/bash

GTESTVERSION=1.7.0
BOOSTVERSION=1_59_0
OPENFSTVERSION=1.5.2

download() {
    if [ ! -e gtest-$GTESTVERSION ]; then
    #   no longer available.  The one in github is not identical (requires cmake)
    #   wget https://googletest.googlecode.com/files/gtest-$GTESTVERSION.zip;
        unzip gtest-$GTESTVERSION.zip ;
    fi
    if [ ! -e openfst-$OPENFSTVERSION ]; then
        wget http://www.openfst.org/twiki/pub/FST/FstDownload/openfst-$OPENFSTVERSION.tar.gz
        tar zxvf openfst-$OPENFSTVERSION.tar.gz
    fi
    if [ ! -e boost_$BOOSTVERSION ]; then
        bv=`echo $BOOSTVERSION | tr '_' '.'`
        wget http://sourceforge.net/projects/boost/files/boost/$bv/boost_$BOOSTVERSION.tar.gz/download
        mv download boost_$BOOSTVERSION.tar.gz
        tar zxvf boost_$BOOSTVERSION.tar.gz
    fi
    # ensure correct permissions always set up.
    chmod -R 755 .
}


installExternals() {
    (
    if [ ! -e gtest-$GTESTVERSION ] || [ ! -e openfst-$OPENFSTVERSION ] || [ ! -e boost_$BOOSTVERSION ]; then
        echo "ERROR: Libraries not downloaded? Check: gtest-$GTESTVERSION openfst-$OPENFSTVERSION boost_$BOOSTVERSION"
        exit
    fi

        if [ ! -e gtest-$GTESTVERSION/INSTALL_DIR ]; then
            cd gtest-$GTESTVERSION;
            ./configure --enable-static --prefix=$PWD/INSTALL_DIR;
            make
# unsupported install for gtest-1.6.0 , gtest-1.7.0 in googletest, no longer available in the github version
#           make install
            mkdir -p INSTALL_DIR/include INSTALL_DIR/lib
            cp -a include/* INSTALL_DIR/include
            cp -a lib/.libs/* INSTALL_DIR/lib
        fi
        )

    (
        if [ ! -e openfst-$OPENFSTVERSION/INSTALL_DIR ]; then
            cd openfst-$OPENFSTVERSION
            ./configure --prefix=$PWD/INSTALL_DIR \
                --enable-bin \
                --enable-compact-fsts \
                --enable-const-fsts \
                --enable-far \
                --enable-lookahead-fsts \
                --enable-python \
                --enable-pdt \
                --enable-static \
                --enable-ngram-fsts
            make
            make install
        fi
        )
 (
        if [ ! -e boost_$BOOSTVERSION/INSTALL_DIR ]; then
            bv=`echo $BOOSTVERSION | tr '_' '.'`
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

# Lattice mert (latmert and lmert tools) can compile with gperftools and Unwind. You need to install
# these libraries on your own and then modify Makefile.inc (which exists _after_ running build-test.sh)
# so that these three variables point to the right places:
# export GPERFTOOLS_INCLUDE=$TOPDIR/externals/gperftools-2.0/INSTALL_DIR/include
# export GPERFTOOLS_LIB=$TOPDIR/externals/gperftools-2.0/INSTALL_DIR/lib
# export UNWIND_LIB=$TOPDIR/externals/libunwind-1.0.1-install/lib
# Afterwards, just do e.g. cd cpp/latmert; make latmert.O2; make install;
# or cd cpp/latmert; make latmert.O2; make install;


### Please write in here the absolute path to HiFST:
export TOPDIR=$TOPDIR

### Default KENLM package -- bundled with HiFST package.
export KENLM_DIR=${TOPDIR}/externals/kenlm/

### To integrate NPLM, 
### 1.- Clone from https://github.com/ucam-smt/nplm.git and install following instructions.
###     This includes installing eigen library
### 2.- Uncomment these variables set them to the appropriate values:
# export NPLM_INCLUDE=...
# export NPLM_LIB=...
# export EIGEN_INCLUDE=...
### And then re-run build-test.sh

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
