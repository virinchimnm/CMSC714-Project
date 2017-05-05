#!/bin/sh

g++ serial_cc.cpp -o ./serial_cc  -std=c++0x 
time ./serial_cc -g ./input/try.txt


mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 
time mpirun -n 2 ./parallel_cc -g ./input/try.txt