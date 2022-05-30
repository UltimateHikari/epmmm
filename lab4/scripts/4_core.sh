#!/bin/bash

#PBS -l walltime=00:30:00
#PBS -l select=1:ncpus=4:mem=2000m,place=pack:excl
#PBS -q xl230g9q
#PBS -m n

cd $PBS_O_WORKDIR

echo $PBS_O_WORKDIR

./scripts/NtimesMthreads.sh lab4s.a 2 4 

