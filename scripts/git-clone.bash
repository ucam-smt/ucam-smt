#!/bin/bash
# Run out of ongoing.hifst, example, run:
# $PWD/ongoing.hifst/scripts/git-clone.bash $PWD/ongoing.hifst gi212-20130112.170215
# This should create and install/test a physical copy of hifst in $PWD/hifst.gi212-20130112.170215
# Note: tag gi212-20130112.170215 must exist first! You can use tag-commit.bash

if [ $# != 2 ] && [ $# != 3 ]; then echo "Usage: $0 dir_to_clone gittag [target_dir]"; exit; fi;

srcdir=$1; shift
tag=$1; shift
if [ $1 ]; then
    tgtdir=$1
else
    tgtdir=$PWD
fi

set -x
if [ -z `(cd $srcdir; git tag -l  | grep $tag )` ]; then echo "Tag does not exist! Create it first"; exit; fi;

if git clone $srcdir $tgtdir/hifst.$tag ; then
    mkdir -p $tgtdir/hifst.$tag # I really want to run commands in this dir!
    cd $tgtdir/hifst.$tag
    mkdir -p bin
    git checkout -b $tag
    ./build-test.sh

    ### dogit -D master #remove master
    ### dogit gc --aggressive #clean .git dir
    ### dogit prune #clean .git dir

fi
