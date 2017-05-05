#!/bin/sh




g++ serial_cc.cpp -o ./serial_cc  -std=c++0x 
time ./serial_cc -g ./input/try.txt


#g++ split_input.cpp -o ./split_input -std=c++0x 
#time ./split_input -g ./input/sample.txt

#mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 
#time mpirun -n 4 ./parallel_cc -g ./input/sample.txt