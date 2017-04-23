#!/bin/sh

WORKDIR=.

echo "Setting up directories .. "
if [ ! -d "$WORKDIR/build" ]; then
  echo "Creating build dir"
  mkdir "$WORKDIR/build"
fi

if [ ! -d "$WORKDIR/bin" ]; then
  echo "Creating bin dir"
  mkdir "$WORKDIR/bin"
fi


g++ ../serial_bfs/src/main.cpp ../serial_bfs/src/graph.h  -std=c++11 -o bin/serial.out
echo "Building Serial BFS done!"


mpic++ -fopenmp -I/usr/local/include -I"$WORKDIR/include" -O0 -g3 -Wall -c -fmessage-length=0 src/*.cpp -std=c++11
mv *.o build
echo "Building dependencies done!"

mpic++ build/DistBFS.o build/BFSUntil.o build/GraphStruct.o -lmpi -lm -std=c++11 -o bin/mpibfs.out
echo "Building MPI BFS done!"

mpic++ -fopenmp build/OmpDistBFS.o  build/BFSUntil.o build/GraphStruct.o -lmpi -lm -std=c++11 -o bin/ompbfs.out
echo "Building OMP-MPI BFS done!"
