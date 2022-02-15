#!/bin/bash

PROFDIR=gprof-profile

mkdir -p "$PROFDIR"
cd "$PROFDIR"
for i in {1..20} 
do
../build/lab1pg.a 10000 10000 100
done