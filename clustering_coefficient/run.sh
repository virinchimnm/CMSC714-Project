#!/bin/sh

#g++ serial_cc.cpp -o ./serial_cc  -std=c++0x 
#./serial_cc -g ./input/sample.txt


mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 
mpirun -n 2 ./parallel_cc -g ./input/try.txt