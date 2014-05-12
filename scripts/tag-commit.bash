#!/bin/bash

# Note: run in ongoing.hifst directory to tag a particular release. 
# You can do: git tag -l to check that the tag you have created

git commit -a -m "Modifications on `date +\"%Y%m%d.%H%M%S\"` -- $1"
git tag  $USER-`date +"%Y%m%d.%H%M%S"`
