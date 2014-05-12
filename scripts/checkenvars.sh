#!/bin/bash

checkVariables() {

    if [[ -d "$OPENFST_INCLUDE"  && ! -z "$OPENFST_INCLUDE" ]] ; then echo "OK -- OPENFST_INCLUDE"; else echo "FAIL -- OPENFST_INCLUDE"; exit; fi
    if [[ -d "$OPENFST_LIB"  && ! -z "$OPENFST_LIB" ]] ; then echo "OK -- OPENFST_LIB"; else echo "FAIL -- OPENFST_LIB"; exit; fi
    if [[ -d "$OPENFST_BIN"  && ! -z "$OPENFST_BIN" ]] ; then echo "OK -- OPENFST_BIN"; else echo "FAIL -- OPENFST_BIN"; exit; fi

    if [[ -d "$BOOST_INCLUDE"  && ! -z "$BOOST_INCLUDE" ]] ; then echo "OK -- BOOST_INCLUDE"; else echo "FAIL -- BOOST_INCLUDE"; exit; fi
    if [[ -d "$BOOST_LIB"  && ! -z "$BOOST_LIB" ]] ; then echo "OK -- BOOST_LIB"; else echo "FAIL -- BOOST_LIB"; exit; fi

    if [[ -d "$GTEST_DIR"  && ! -z "$GTEST_DIR" ]] ; then echo "OK -- GTEST_DIR"; else echo "FAIL -- GTEST_DIR"; exit; fi
    if [[ -d "$GTEST_INCLUDE"  && ! -z "$GTEST_INCLUDE" ]] ; then echo "OK -- GTEST_INCLUDE"; else echo "FAIL -- GTEST_INCLUDE"; exit; fi
    if [[ -d "$GTEST_LIB"  && ! -z "$GTEST_LIB" ]] ; then echo "OK -- GTEST_LIB"; else echo "FAIL -- GTEST_LIB"; exit; fi
    if [[ -d "$TOPDIR"  && ! -z "$TOPDIR" ]] ; then echo "OK -- TOPDIR"; else echo "FAIL -- TOPDIR"; exit; fi
    if [[ -d "$KENLM_DIR"  && ! -z "$KENLM_DIR" ]] ; then echo "OK -- KENLM_DIR"; else echo "FAIL -- KENLM_DIR"; exit; fi

}




