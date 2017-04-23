#!/bin/sh
WORKDIR=.
mpicc -fopenmp -I/usr/local/include -I"$WORKDIR/include" -O0 -g3 -Wall -c -fmessage-length=0 src/*.c
mv *.o build
mpicc -fopenmp build/DistBFS.o build/ArrayList.o build/BFSUntil.o build/GraphStruct.o -o ompBFS -lmpi -lm
