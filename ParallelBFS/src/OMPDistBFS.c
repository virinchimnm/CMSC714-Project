#include <time.h>
#include <limits.h>
#include <mpi.h>
#include "BFS.h"
#include "ArrayList.h"
#include <omp.h>

static int myRank;

int* BFS(MPI_Comm comm, GraphStruct localGraph, int srcLid, int srcRank)
{
	// omp_set_num_threads(omp_n); Should be passed from the argument list
	int i, j;
	/************** Compute the mapping of vertex GID to vertex lid *****************/
	int totalVtx;
	MPI_Allreduce(&(localGraph.numVertices), &totalVtx, 1, MPI_INT, MPI_SUM, comm);
	
	// TODO: Replace gid2lid by an unordered_map
	int *gid2lid = (int *) calloc(sizeof(int), totalVtx);

	#pragma omp parallel for private(i)
	for(i=0; i<localGraph.numVertices; i++)
	{
		int gid = localGraph.vertexGIDs[i];
		gid2lid[gid] = i;
	}

	/******* Initialize sending and receiving buffer ********/
	int *recvCount = (int *) malloc(sizeof(int) * localGraph.numParts);
	int **recvBuf = (int **) malloc(sizeof(int *) * localGraph.numParts);
	unsigned long *recvDummy = (unsigned long *) malloc(sizeof(unsigned long) * localGraph.numParts);
	unsigned long *sendDummy = (unsigned long *) malloc(sizeof(unsigned long) * localGraph.numParts);
	
	// TODO: Replace ArrayList by vector
	ArrayList_t **sendBuf = (ArrayList_t **) malloc(sizeof(ArrayList_t *) * localGraph.numParts);
	
	#pragma omp parallel for
	for(i=0; i<localGraph.numParts; i++)
	{
		sendBuf[i] = listCreate();
	}

	/*****************************************************************/
	// printf("MyRank, SrcLid, SrcRank %d %d %d\n", myRank, srcLid, srcRank);
	//printf("MyRank, localGraph.numVertices %d %d\n", myRank, localGraph.numVertices);
	int *d = (int *) malloc(sizeof(int)* localGraph.numVertices);
	
	#pragma omp parallel for
	for(int i=0; i<localGraph.numVertices; i++)
		d[i] = -1;
	
	// Replace by vector
	ArrayList_t * FS = listCreate();	//vertices currently active
	if(srcRank == myRank)
	{
		d[srcLid] = 0;
		listAppend(FS, srcLid);
	}

	int level = 1;
	int numActiveVertices = 0;
	do
	{
		//visiting neighbouring vertices in parallel
		ArrayList_t * NS = listCreate();	//vertices active in the next computation step
		memset(sendDummy, 0, sizeof(unsigned long) * localGraph.numParts);

		#pragma omp parallel for private(i)
		for(i=0; i<listLength(FS); i++)
		{
			int lid = listGetIdx(FS, i);

			// Iterate over the neighbours of the vertex
			#pragma omp parallel for firstprivate(myRank) //not sure if we need first private
			for(j=localGraph.nborIndex[lid]; j<localGraph.nborIndex[lid + 1]; j++)
			{
				// This is the Global ID of the neighbour
				int nborGID = localGraph.nborGIDs[j];
				// This is the processor on which the neighbour is sitting on
				int owner = localGraph.nborProcs[j];

				// If I am the owner, find the local ID of this node and append to
				// neighbour frontier.
				if(owner == myRank)
				{
					int lid = gid2lid[nborGID];
					if(d[lid] == -1){
						#pragma omp critical // works without it!
						listAppend(NS, lid);
						d[lid] = level;
					}
				}
				else
				{
					// Append to the list of the owner about this node.
					#pragma omp critical // works without it!
					listAppend(sendBuf[owner], nborGID);
				}
			}
		}

		listDestroy(FS);
		FS = NS;

		MPI_Request request;
		//sending newly visited nbors to their owners in parellel
		#pragma omp parallel for //not sure if we need first private
		for(i=0; i<localGraph.numParts; i++)
		{
			// Sending the length of the send buffer to the neighbours
			if(sendBuf[i]->length)
			{
				MPI_Isend(sendBuf[i]->data, sendBuf[i]->length, MPI_INT, i, 1, comm, &request);
				MPI_Request_free(&request);
			}
		}

//->		//#pragma omp parallel for private(i) THIS CREATES SEGMENTATION FAULT!
		for(i=0; i<localGraph.numParts; i++)
		{
			// Rank i gather sendCount[i] from each rank
			// This is the total number of neighbours this node gets
			MPI_Gather(&(sendBuf[i]->length), 1, MPI_INT, recvCount, 1, MPI_INT, i, comm);
			MPI_Gather(sendDummy + i, 1, MPI_UNSIGNED_LONG, recvDummy, 1, MPI_UNSIGNED_LONG, i, comm);
		}

		for(i=0; i<localGraph.numParts; i++)
		{
			recvBuf[i] = (int *) malloc(sizeof(int) * recvCount[i]);
			if(recvCount[i])
			{
				MPI_Recv(recvBuf[i], recvCount[i], MPI_INT, i, 1, comm, MPI_STATUS_IGNORE);
			}
		}

		//handling newly visited vertices and compute the distance
		
		#pragma omp parallel for
		for(i=0; i<localGraph.numParts; i++)
		{
			#pragma omp parallel for private(j)
			for(j=0; j<recvCount[i]; j++)
			{
				int gid = recvBuf[i][j];
				int lid = gid2lid[gid];
				if(d[lid] == -1)
				{
					d[lid] = level;
					#pragma omp critical 
					listAppend(FS, lid);
				}
			}
			free(recvBuf[i]);
		}

		numActiveVertices = listLength(FS);
		MPI_Allreduce(MPI_IN_PLACE, &numActiveVertices, 1, MPI_INT, MPI_SUM, comm);
		
		#pragma omp parallel for
		for(i=0; i<localGraph.numParts; i++){
			listClear(sendBuf[i]);
		}
//		if(myRank == 0) printf("step-%d\n", level);
		level ++;
	} while(numActiveVertices > 0);

	free(gid2lid);
	free(sendDummy);
	free(recvDummy);

	#pragma omp parallel for
	for(i=0; i<localGraph.numParts; i++){
		listDestroy(sendBuf[i]);
	}
	free(sendBuf);
	free(recvBuf);
	free(recvCount);
	//printf("myRank %d Done\n", myRank);
	return d;
}

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	// char *fname = "../CMSC714-Project/datasets/com-youtube.ungraph-final-input.txt";
	// char *fname = "graph2.txt";
	 char *fname = "sample_input.txt";
	//char *fname = "youtube-4.txt";

	int numParts;
	MPI_Comm_rank(comm, &myRank);
	MPI_Comm_size(comm, &numParts);


	GraphStruct localGraph;

	// TODO: Maybe we should be able to replace this by graphLoad(&localGraph, subGraphFile)
	getSubGraph(comm, &localGraph, fname, numParts);
	srand(time(NULL));
	// int srcRank = rand() % numParts;
	// int srcLid = rand() % localGraph.numVertices;
	int srcRank = 0;
	int srcLid = 0;
	int *d = BFS(comm, localGraph, srcLid, srcRank);

	int i;
	int totalVtx;
	MPI_Allreduce(&(localGraph.numVertices), &totalVtx, 1, MPI_INT, MPI_SUM, comm);
	int *globalDist = (int *) calloc(sizeof(int), totalVtx);
	for(i=0; i<localGraph.numVertices; i++){
		 globalDist[localGraph.vertexGIDs[i]] = d[i];
	}
	free(d);
	MPI_Allreduce(MPI_IN_PLACE, globalDist, totalVtx, MPI_INT, MPI_MAX, comm);
	if(myRank == srcRank){
		for(i=0; i<totalVtx; i++){
			printf("d[%d] = %d\n", i, globalDist[i]);
		}
	}
	free(globalDist);
	graphDeinit(&localGraph);
	MPI_Finalize();
	return 0;
}
