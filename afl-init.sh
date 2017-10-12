#!/bin/bash

mkdir -p afl-in
mkdir -p afl-out

cd afl-in
split -l1 ../tmp/in

# run afl with the following command:
# afl-fuzz -i afl-in -o afl-out -M master0 -x afl-dict.txt  -- ./cnitize-afl
