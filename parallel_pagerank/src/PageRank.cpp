#include <time.h>
#include <limits.h>
#include <mpi.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include "GraphDist.h"
// #include <omp.h>

// #define WRITE_OUTPUT

typedef unsigned int uint;

static int myRank;
static clock_t exec_time;
static double d = 0.85;
static int num_iter = 10;

using namespace std;

void compute_degree(MPI_Comm comm, GraphStruct* localGraph)
{
	int totalVtx;
	MPI_Allreduce(&(localGraph->numVertices), &totalVtx, 1, MPI_INT, MPI_SUM, comm);
	localGraph->N = totalVtx;

	for(int i =0; i<localGraph->numVertices; i++)
	{
		int gid = localGraph->vertexGIDs[i];
		localGraph->gid2lid[gid] = i;
	}

	int last = localGraph->numVertices - 1;

	for(int i =0; i<localGraph->numVertices; i++)
	{
		// Iterate over the neighbours of the vertex
		// for(int j=localGraph->nborIndex[lid]; j<localGraph->nborIndex[lid + 1]; j++)

		localGraph->degree[i] = localGraph->nborIndex[i + 1] - localGraph->nborIndex[i];
		localGraph->status[i] = 0;
		localGraph->oldPageRank[i] = 1.0 / ((double) localGraph->N);

		for(int j=localGraph->nborIndex[i]; j<localGraph->nborIndex[i + 1]; j++)
		{
			int owner = localGraph->nborProcs[j];
			int nborGID = localGraph->nborGIDs[j];
			if (owner != myRank)
			{
				localGraph->status[i] = 1;
				// break;
				localGraph->externalContributions[nborGID] = 0.0;
			}
		}

		if(localGraph->status[i] == 0)
			localGraph->opt_map[localGraph->numTrulyLocal++] = i;
		else
			localGraph->opt_map[last--] = i;
	}

	MPI_Barrier(comm);
}

void pageRank(MPI_Comm comm, GraphStruct* localGraph, int num_iter)
{
	// Number of vertices who have atleat one non local neighbours
	int numNonLocal = localGraph->numVertices - localGraph->numTrulyLocal;
	vector<double> sendBuf(numNonLocal);
	vector<int> sendGids(numNonLocal);

	vector<int> recvCount(localGraph->numParts);
	vector<vector<double> > recvBuf(localGraph->numParts);
	vector<vector<int> > recvGids(localGraph->numParts);

	double *swap_temp;

	for(int iter=0; iter < num_iter; iter++)
	{
		// printf("Checkpoint 1 %d\n", myRank);
		// Compute pagerank/degree for nodes which are neighbours of vertices on different nodes
		// TODO: OpenMP here.
		for(int k=0; k<numNonLocal; k++)
		{
			int i = localGraph->opt_map[k + localGraph->numTrulyLocal];
			sendBuf[k] = localGraph->oldPageRank[i]/localGraph->degree[i];
			sendGids[k] = localGraph->vertexGIDs[i];
		}

		// printf("Checkpoint 2 %d\n", myRank);
		// Send GIDs, PR/degree of non-truly vertices to everyone.
		// TODO: Can optimize so that only the ones who need it receive it.
		MPI_Request request, request2;
		for(int i=0; i<localGraph->numParts; i++)
		{
			if(myRank != i)
			{
				MPI_Isend(&(sendBuf.front()), numNonLocal, MPI_DOUBLE, i, 1, comm, &request);
				MPI_Isend(&(sendGids.front()), numNonLocal, MPI_INT, i, 2, comm, &request2);
				MPI_Request_free(&request);
				MPI_Request_free(&request2);
			}
		}

		// printf("Checkpoint 3 %d\n", myRank);
		// Compute page rank for truly local vertices (overlapping with the communication)
		// TODO: OpenMP here.
		for(int k=0; k< localGraph->numTrulyLocal; k++)
		{
			int i = localGraph->opt_map[k];

			localGraph->currPageRank[i] = (1-d)/((double)localGraph->N);
			for(int j=localGraph->nborIndex[i]; j<localGraph->nborIndex[i + 1]; j++)
			{
				int nborGID = localGraph->nborGIDs[j];
				int lid = localGraph->gid2lid[nborGID];

				localGraph->currPageRank[i] += d * (localGraph->oldPageRank[lid] / ((double) localGraph->degree[lid]));

			}
		}

		// MPI_Barrier(comm);

		// printf("Checkpoint 4 %d\n", myRank);
		for(int i=0; i<localGraph->numParts; i++)
		{
			// Rank i gather sendCount[i] from each rank
			// This is the total number of neighbours this node gets
			int s = sendBuf.size();
			if(i == myRank)
				s = 0;
			MPI_Gather(&(s), 1, MPI_INT, &recvCount.front(), 1, MPI_INT, i, comm);
		}

		// printf("Checkpoint 5 %d\n", myRank);
		// Get info from neighbours
		for(int i=0; i<localGraph->numParts; i++)
		{
			recvBuf[i] = std::vector<double>(recvCount[i]);
			recvGids[i] = std::vector<int>(recvCount[i]);
			if(recvCount[i])
			{
				MPI_Recv(&(recvBuf[i].front()), recvCount[i], MPI_DOUBLE, i, 1, comm, MPI_STATUS_IGNORE);
				MPI_Recv(&(recvGids[i].front()), recvCount[i], MPI_INT, i, 2, comm, MPI_STATUS_IGNORE);
			}
		}

		// printf("Checkpoint 6 %d\n", myRank);
		// Process the incoming info and keep only what you require.
		// TODO: OpenMP here.
		for(int i=0; i<localGraph->numParts; i++)
		{
			for(int j=0; j<recvCount[i]; j++)
			{
				// Update the map. This is not an insert operation, so could happen thread safe?
				if(localGraph->externalContributions.find(recvGids[i][j]) != localGraph->externalContributions.end())
				{
					localGraph->externalContributions[recvGids[i][j]] = recvBuf[i][j];
				}
			}
		}

		// printf("Checkpoint 7 %d\n", myRank);
		// Compute page rank on status = 1 nodes
		// TODO: OpenMP here.
		for(int k=0; k<numNonLocal; k++)
		{
			int i = localGraph->opt_map[k + localGraph->numTrulyLocal];
			
			localGraph->currPageRank[i] = (1-d)/((double)localGraph->N);
			for(int j=localGraph->nborIndex[i]; j<localGraph->nborIndex[i + 1]; j++)
			{
				int owner = localGraph->nborProcs[j];
				int nborGID = localGraph->nborGIDs[j];
				if(owner == myRank)
				{
					int lid = localGraph->gid2lid[nborGID];	
					localGraph->currPageRank[i] += d * (localGraph->oldPageRank[lid] / ((double) localGraph->degree[lid]));
				}
				else
				{
					localGraph->currPageRank[i] += d * localGraph->externalContributions[nborGID];
				}
			}
		}

		swap_temp = localGraph->currPageRank;
		localGraph->currPageRank = localGraph->oldPageRank;
		localGraph->oldPageRank = swap_temp;

		MPI_Barrier(comm);
		// cout << "Rank " << myRank << " iter " << iter << endl;
	}
}

void parseCommandLineArguments(int argc,char *argv[], int &root, std::string &ip, std::string &op)
{
	ip = "input/sample_input.txt";
	op = "";
	for(int i = 1; i < argc; i++)
	{
		if(std::string(argv[i]) == "-i")
		{
			ip = std::string(argv[i+1]);
			std::cout << "input file path " << ip << std::endl;
		}
		else if(std::string(argv[i]) == "-iter")
		{
			num_iter = std::stoi(argv[i+1]);
			std::cout << "Number of iterations " << num_iter << std::endl;
		}
	}
}


int main(int argc, char *argv[]) {

	int numParts;
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	MPI_Comm_size(comm, &numParts);

	if(myRank == 0)
		cout << "Number of MPI processes " << numParts << endl;
	
	int srcRank = 0;
	int srcLid = 0;
	std::string fname, ofname;

	if(myRank == srcRank)
	{
		parseCommandLineArguments(argc,argv,srcLid,fname,ofname);
	}

	GraphStruct localGraph;

	// TODO: Maybe we should be able to replace this by graphLoad(&localGraph, subGraphFile)
	getSubGraph(comm, &localGraph, fname, numParts);

	clock_t start = clock();
	compute_degree(comm, &localGraph);
	pageRank(comm, &localGraph, num_iter);

	clock_t stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);
	// printf("BEFORE FINAL BARRIER %d\n", myRank);
	MPI_Barrier(comm);

#ifdef WRITE_OUTPUT
	for(int i =0; i<localGraph.numVertices; i++)
	{
		printf("%d %.6lf\n", localGraph.vertexGIDs[i], localGraph.oldPageRank[i]);
	}
#endif // WRITE_OUTPUT
	cout << exec_time <<"ms (mpi)"<< endl;

	graphDeinit(&localGraph);
	MPI_Finalize();
	return 0;
}
