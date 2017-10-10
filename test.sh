#!/bin/bash

mkdir tmp 2>/dev/null

cut -d '$' -f1 tests.txt > tmp/in
cut -d '$' -f2 tests.txt > tmp/expected
./cnitize < tmp/in > tmp/out

diff -u tmp/expected tmp/out
