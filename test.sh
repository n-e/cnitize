#!/bin/bash

mkdir tmp 2>/dev/null

cut -d '$' -f1 tests.txt > tmp/in
cut -d '$' -f2 tests.txt > tmp/expected
./cnitize < tmp/in > tmp/out

diff -y --suppress-common-lines tmp/out tmp/expected 

FAIL=`diff --suppress-common-lines --speed-large-files -y tmp/out tmp/expected |wc -l |sed 's/[^0-9]//g'`
ALL=`cat tmp/expected|wc -l |sed 's/[^0-9]//g'`
let "SUCCESS = $ALL - $FAIL"
echo "Succeeded: $SUCCESS/$ALL"
