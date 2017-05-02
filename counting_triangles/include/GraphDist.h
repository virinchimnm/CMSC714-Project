#ifndef GRAPHDIST_H_
#define GRAPHDIST_H_

#include <mpi.h>
#include "GraphStruct.h"
#include <vector>

void graphDistribute(MPI_Comm comm, GraphStruct global_graph, GraphStruct * local_graph);
void getSubGraph(MPI_Comm comm, GraphStruct *sub_graph, std::string fname, int numParts);



#endif /* GRAPHDIST_H_ */
