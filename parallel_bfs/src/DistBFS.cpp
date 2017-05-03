#include <time.h>
#include <limits.h>
#include <mpi.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include "BFS.h"
// #include <omp.h>

typedef unsigned int uint;

static int myRank;
static clock_t exec_time;

using namespace std;

std::vector<int> BFS(MPI_Comm comm, GraphStruct localGraph, int srcLid, int srcRank)
{
	// omp_set_num_threads(omp_n); Should be passed from the argument list
	/************** Compute the mapping of vertex GID to vertex lid *****************/
	int totalVtx;
	MPI_Allreduce(&(localGraph.numVertices), &totalVtx, 1, MPI_INT, MPI_SUM, comm);
	
	// TODO: Replace gid2lid by an unordered_map

	std::unordered_map<int,int> gid2lid;

	for(int i =0; i<localGraph.numVertices; i++)
	{
		int gid = localGraph.vertexGIDs[i];
		gid2lid[gid] = i;
	}

	/******* Initialize sending and receiving buffer ********/
	std::vector<int> recvCount(localGraph.numParts);
	std::vector<unsigned long> recvDummy(localGraph.numParts);
	std::vector<unsigned long> sendDummy(localGraph.numParts);
	
	std::vector<std::vector<int>> recvBuf(localGraph.numParts);
	std::vector<std::vector<int>> sendBuf(localGraph.numParts);

	std::vector<int> dist(localGraph.numVertices, -1);
	std::vector<int> *FS = new std::vector<int>(0);
	
	if(srcRank == myRank)
	{
		dist[srcLid] = 0;
		FS->push_back(srcLid);
	}

	int level = 1;
	int numActiveVertices = 0;
	do
	{
		//visiting neighbouring vertices in parallel
		std::vector<int> *NS = new std::vector<int>(0);
		sendDummy.assign(localGraph.numParts,0);
		
		//#pragma omp parallel for private(i)
		for(uint i=0; i<FS->size(); i++)
		{
			int lid = FS->at(i);

			// Iterate over the neighbours of the vertex
			for(int j=localGraph.nborIndex[lid]; j<localGraph.nborIndex[lid + 1]; j++)
			{
				// This is the Global ID of the neighbour
				int nborGID = localGraph.nborGIDs[j];
				// This is the processor on which the neighbour is sitting on
				int owner = localGraph.nborProcs[j];

				// If I am the owner, find the local ID of this node and append to neighbour frontier.
				if(owner == myRank)
				{
					int lid = gid2lid[nborGID];
					if(dist[lid] == -1){
						NS->push_back(lid);
						dist[lid] = level;
					}
				}
				else
				{
					sendBuf[owner].push_back(nborGID);
				}
			}
		}

		FS->clear();
		FS = NS;

		MPI_Request request;
		//sending newly visited nbors to their owners in parallel
		for(int i=0; i<localGraph.numParts; i++)
		{
			// Sending the length of the send buffer to the neighbours
			if(sendBuf[i].size())
			{
				MPI_Isend(&(sendBuf[i].front()), sendBuf[i].size(), MPI_INT, i, 1, comm, &request);
				MPI_Request_free(&request);
			}
		}


		for(int i=0; i<localGraph.numParts; i++)
		{
			// Rank i gather sendCount[i] from each rank
			// This is the total number of neighbours this node gets
			int s = sendBuf[i].size();
			MPI_Gather(&(s), 1, MPI_INT, &recvCount.front(), 1, MPI_INT, i, comm);
			MPI_Gather(&sendDummy.front() + i, 1, MPI_UNSIGNED_LONG, &recvDummy.front(), 1, MPI_UNSIGNED_LONG, i, comm);
		}

		for(int i=0; i<localGraph.numParts; i++)
		{
			recvBuf[i] = std::vector<int>(recvCount[i]);
			if(recvCount[i])
			{
				MPI_Recv(&(recvBuf[i].front()), recvCount[i], MPI_INT, i, 1, comm, MPI_STATUS_IGNORE);
			}
		}

		//handling newly visited vertices and compute the distance
		
		for(int i=0; i<localGraph.numParts; i++)
		{
			for(int j=0; j<recvCount[i]; j++)
			{
				int gid = recvBuf[i][j];
				int lid = gid2lid[gid];
				if(dist[lid] == -1)
				{
					dist[lid] = level;
					FS->push_back(lid);
				}
			}
			recvBuf[i].clear(); // is this required?
		}

		numActiveVertices = FS->size();
		MPI_Allreduce(MPI_IN_PLACE, &numActiveVertices, 1, MPI_INT, MPI_SUM, comm);
		
		for(int i=0; i<localGraph.numParts; i++)
		{
			sendBuf[i].clear();
		}

		level ++;
	} while(numActiveVertices > 0);

	sendBuf.clear();

	return dist;
}

void parseCommandLineArguments(int argc,char *argv[], int &root, std::string &ip, std::string &op)
{
	root = 0;
	ip = "input/sample_input.txt";
	op = "";
	for(int i = 1; i < argc; i++)
	{
		//cout << (string(argv[i]) == "-o") << endl;
		if(std::string(argv[i]) == "-r")
		{
			root = std::stoi(argv[i+1]);
			std::cout << "root set to " << root << std::endl;
		}
		else if(std::string(argv[i]) == "-i")
		{
			ip = std::string(argv[i+1]);
			std::cout << "input file path " << ip << std::endl;
		}
		else if(std::string(argv[i]) == "-o")
		{
			op = std::string(argv[i+1]);
			std::cout << "output file path " << op << std::endl;
		}
	}
}

void saveBFSTree(std::string &op, std::vector<int> &dist)
{
	std::ofstream myfile;
	myfile.open (op, std::ofstream::out | std::ofstream::trunc);
	int i = 0;
	for(const int d: dist)
			myfile << "d[" << i++ << "] = " << d << std::endl;
	myfile << exec_time <<"ms (mpi)"<<std::endl;
	myfile.close();						
}



int main(int argc, char *argv[]) {

	int numParts;
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	MPI_Comm_size(comm, &numParts);

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
	srand(time(NULL));


	int num_iter = 100;
	clock_t start = clock();
    std::vector<int> dist = BFS(comm, localGraph, srcLid, srcRank);
    for(int i=0; i<num_iter; i++)
	   BFS(comm, localGraph, srcLid, srcRank);
	clock_t stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);
    exec_time = exec_time / (double(num_iter+1));

	int i;
	int totalVtx;
	MPI_Allreduce(&(localGraph.numVertices), &totalVtx, 1, MPI_INT, MPI_SUM, comm);

	std::vector<int> globalDist(totalVtx, -1);
	for(i=0; i<localGraph.numVertices; i++){
		 globalDist[localGraph.vertexGIDs[i]] = dist[i];
	}
	
	MPI_Allreduce(MPI_IN_PLACE, &(globalDist.front()), totalVtx, MPI_INT, MPI_MAX, comm);
	if(myRank == srcRank)
	{
		if(ofname == "")
		{
			for(i=0; i<totalVtx; i++)
			{
			 	std::cout << "d[" << i << "] = " << globalDist[i] << std::endl;
			}
		}
		else
		{
			saveBFSTree(ofname,globalDist);
			std::cout << "output saved to " << ofname << std::endl;
		}
	}

	globalDist.clear();
	graphDeinit(&localGraph);
	MPI_Finalize();
	return 0;
}
