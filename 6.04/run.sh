#!/bin/bash
set -e
mpic++ source.cpp -o program.o -std=c++11
mpirun -n $1 ./program.o $2
