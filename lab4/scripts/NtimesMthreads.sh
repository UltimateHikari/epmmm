#!/bin/bash

PROFDIR=gprof-profile
TIMES=$2
PROCS=$3

mkdir -p "$PROFDIR"
cd "$PROFDIR"
for i in 1 2 4 8 10 16 20 30
do
    echo "----$i-400-------------------------------"
    for j in {1..$TIMES} 
    do
        ../build/$1 10000 10000 400 $i $PROCS
    done
done

for i in 6 12 18 24
do
    echo "----$i--360------------------------------"
    for j in {1..$TIMES} 
    do
        ../build/$1 10000 10000 360 $i $PROCS
    done
done

echo "----14--420------------------------------"
for j in {1..$TIMES} 
do
    ../build/$1 10000 10000 420 $i $PROCS
done

echo "----22--440------------------------------"
for j in {1..$TIMES} 
do
    ../build/$1 10000 10000 440 $i $PROCS
done

echo "----26--520------------------------------"
for j in {1..10} 
do
    ../build/$1 10000 10000 520 $i $PROCS
done

echo "----28--560------------------------------"
for j in {1..$TIMES} 
do
    ../build/$1 10000 10000 560 $i $PROCS
done
