#!/bin/bash

mkdir -p afl-in
mkdir -p afl-out

cd afl-in
split -l1 ../tmp/in
