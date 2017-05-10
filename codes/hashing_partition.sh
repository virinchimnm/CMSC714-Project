#!/bin/sh

#######
### Usage:
### ./run.sh input_graph_downloaded num_part output_file_desired_BFS


echo "Transforming to 0-based indexing"
g++ scale_graph_to_zero.cpp -o scale_graph_to_zero
./scale_graph_to_zero -g $1 > scaled.out

echo "Transforming to input required by BFS"
g++ hashing_partition.cpp -o hashing_partition
./hashing_partition -g scaled.out -p $2 > $3

echo "Writing the output to file $3"