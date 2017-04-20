#!/bin/sh

### transform to zero_based_indexing of node ids
#g++ scale_graph_to_zero.cpp -o scale_graph_to_zero
#./scale_graph_to_zero -g ./data/in.txt > ./data/transformed.txt

### transform to zero_based_indexing of node ids
#g++ output_to_gpmetis.cpp -o output_to_gpmetis
#./output_to_gpmetis -g ./data/scaled.txt > ./data/gp_format.txt

### gpmetis usage to get
#gpmetis path-to-input-graph no-of-partitions

### transform the output as needed by bfs
#g++ output_to_graph_algo.cpp -o output_to_graph_algo
#./output_to_graph_algo -g ./data/transformed.txt -p ./data/gp_format.txt.part.2
