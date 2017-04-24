#ifndef BFS_H_
#define BFS_H_

#include <mpi.h>
#include "GraphStruct.h"
#include <vector>

void graphDistribute(MPI_Comm comm, GraphStruct global_graph, GraphStruct * local_graph);
void getSubGraph(MPI_Comm comm, GraphStruct *sub_graph, std::string fname, int numParts);



#endif /* BFS_H_ */
