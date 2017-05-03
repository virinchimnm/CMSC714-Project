#ifndef GRAPHSTRUCT_H_
#define GRAPHSTRUCT_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unordered_map>
#include <vector>

typedef struct {
	int numParts;				/* total number of partitions of entire graph */
	int numVertices; 			/* total number of vertices own by the MPI repartitioning process */
	int numNbors; 				/* total number of neighbors of my vertices */

	int *vertexGIDs; 			/* global ID of each vertex */
	int *partVector;			/* partition vector of vertex */
	int *degree;				/* Degree of this vertex */
	int *status;				/* 0: all neighbours are local, 1: atleast one neighbour is non local */

	int *nborIndex; 			/* nborIndex[i] is location of the first neighbor of vertex i,  (nbor = nborIndex[i + 1] -  nborIndex[i]) equals to the number of neighbors of vertex i*/
	int *nborGIDs; 				/* nborGIDs[nborIndex[i], ...., nborIndex[i] + nbor - 1] are neighbors of vertex i */
	int *nborProcs; 			/* the rank number of the MPI process owning the neighbor in nborGID. */

	double *pageRank;
	double *currPageRank, *oldPageRank;

	std::unordered_map<int,int> gid2lid;

	std::unordered_map<int, int> opt_map;
	int numTrulyLocal;

	std::unordered_map<int, float> externalContributions; /* The contributions needed by this node from vertices of other nodes */

	int N; 						/* Total number of vertices in the full graph */

} GraphStruct;

void graphInit(GraphStruct *graph, int num_vtxs, int num_edges, int num_parts);
void graphDeinit(GraphStruct *graph);
void graphPrint(GraphStruct graph);

int graphLoad(GraphStruct *global_graph, FILE * gdata);


#endif /* GRAPHSTRUCT_H_ */
