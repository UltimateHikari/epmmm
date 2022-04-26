#!/bin/bash

PROFDIR=gprof-profile

mkdir -p "$PROFDIR"
cd "$PROFDIR"
for i in {1..10} 
do
../build/$1 10000 10000 100
done