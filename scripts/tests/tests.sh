#!/bin/bash

. ../../Makefile.inc
. ../checkenvars.sh
checkVariables

for testscript in `ls test-*.sh`; do ./$testscript; done | tee test.log

passed=`echo \`cat test.log | grep PASSED | awk '{print $2}'\` | sed -e 's: :+:g' | bc`
failed=`echo \`cat test.log | grep FAILED | awk '{print $2}'\` | sed -e 's: :+:g' | bc`
skip=`echo \`cat test.log | grep SKIPPED | awk '{print $2}'\` | sed -e 's: :+:g' | bc`
total=`echo \`cat test.log | grep TOTAL | awk '{print $2}'\` | sed -e 's: :+:g' | bc`

echo "============================================================"
echo "TESTS FINISHED. OVERALL RESULTS...:"
printf "PASSED : %5d tests \n" $passed 
printf "FAILED : %5d tests \n" $failed 
printf "SKIPPED: %5d tests \n" $skip
printf "TOTAL  : %5d tests \n" $total 
