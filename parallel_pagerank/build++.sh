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


# g++ ../serial_bfs/src/main.cpp ../serial_bfs/src/graph.h  -std=c++0x -o bin/serial.out
# echo "Building Serial BFS done!"


# mpic++ -fopenmp -I/usr/local/include -I"$WORKDIR/include" -O0 -g3 -Wall -c -fmessage-length=0 src/*.cpp -std=c++0x
mpic++  -I/usr/local/include -I"$WORKDIR/include" -O0 -g3 -Wall -c -fmessage-length=0 src/*.cpp -std=c++0x
mv *.o build
echo "Building dependencies done!"

mpic++ build/PageRank.o build/Util.o build/GraphStruct.o -lmpi -lm -std=c++0x -o bin/mpiPageRank.out
echo "Building MPI PageRank done!"

# mpic++ -fopenmp build/OmpDistBFS.o  build/BFSUntil.o build/GraphStruct.o -lmpi -lm -std=c++0x -o bin/ompbfs.out
# echo "Building OMP-MPI BFS done!"
