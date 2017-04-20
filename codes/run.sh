#!/bin/sh

#######
### Usage:
### ./run.sh input_graph_downloaded num_part output_file_desired_BFS

### transform to zero_based_indexing of node ids
echo "Transforming to 0-based indexing"
g++ scale_graph_to_zero.cpp -o scale_graph_to_zero
./scale_graph_to_zero -g $1 > scaled.out

## transform to one_based_indexing of node ids
echo "Transforming to gmetis required input"
g++ output_to_gpmetis.cpp -o output_to_gpmetis
./output_to_gpmetis -g scaled.out > gp.out

### gpmetis usage to get number of partitions
gpmetis gp.out $2

### transform the output as needed by bfs
echo "Transforming to input required by BFS"
g++ output_to_graph_algo.cpp -o output_to_graph_algo
./output_to_graph_algo -g scaled.out -p gp.out.part.$2 > $3

echo "Writing the output to file $3"
