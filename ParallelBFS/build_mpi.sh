#!/bin/sh
WORKDIR=.
mpicc -I/usr/local/include -I"$WORKDIR/include" -O0 -g3 -Wall -c -fmessage-length=0 src/ArrayList.c src/BFSUntil.c src/DistBFS.c src/GraphStruct.c 
mv *.o build
mpicc build/DistBFS.o build/ArrayList.o build/BFSUntil.o build/GraphStruct.o -o BFS -lmpi -lm
# mpicc -fopenmp build/OMPDistBFS.o build/ArrayList.o build/BFSUntil.o build/GraphStruct.o -o ompBFS -lmpi -lm
