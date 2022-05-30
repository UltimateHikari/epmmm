#!/bin/bash

./scripts/NtimesMthreads.sh lab4.a 2 1 > ./timetest_1
./scripts/NtimesMthreads.sh lab4.a 2 2 > ./timetest_2
./scripts/NtimesMthreads.sh lab4.a 2 4 > ./timetest_4
./scripts/NtimesMthreads.sh lab4.a 4 8 > ./timetest_8
./scripts/NtimesMthreads.sh lab4.a 4 10 > ./timetest_10