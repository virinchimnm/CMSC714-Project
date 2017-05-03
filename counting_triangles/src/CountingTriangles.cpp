#include <time.h>
#include <limits.h>
#include <mpi.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <GraphDist.h>

typedef unsigned int uint;

static int myRank;

using namespace std;

int intersections(std::vector<int> &v, std::unordered_set<int> &set){
	int common = 0;
	for(int i=0; i<v.size(); i++){
		if(set.find(v[i]) == set.end()){
			continue;
		}
		common++;
	}
	return common;
}

int CountingTriangles(MPI_Comm comm, GraphStruct localGraph, int srcRank){

	//Need to select a node from the frontier. And then start the process.

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

	std::vector<std::vector<int>> recvAdjacency(localGraph.numParts);
	std::vector<std::vector<int>> sendAdjacency(localGraph.numParts);

	std::vector<std::vector<int>> recvAdjacencyLengths(localGraph.numParts);
	std::vector<std::vector<int>> sendAdjacencyLengths(localGraph.numParts);

	std::vector<int> *localNeighbours = new std::vector<int>(0);

	int localTri = 0, remoteTri=0; 

	// std::vector<int> *localFrontier = new std::vector<int>(0);

	//Start with the local id 0 of the graph.
	int activeThread = 1;
	int iteration = 1;
	int currentNodeGid = 0;

	MPI_Allreduce(MPI_IN_PLACE, &activeThread, 1, MPI_INT, MPI_SUM, comm);

	printf("Process: %d ; activeThread : %d\n", myRank, activeThread);

	while(activeThread > 0){

		std::unordered_set<int> currentNodeSet;

		if(currentNodeGid != -1){
			for(int j=localGraph.nborIndex[iteration]; j<localGraph.nborIndex[iteration + 1]; j++)
			{
				// This is the Global ID of the neighbour
				int nborGID = localGraph.nborGIDs[j];
				currentNodeSet.insert(nborGID);
				// This is the processor on which the neighbour is sitting on
				int owner = localGraph.nborProcs[j];

				// If I am the owner, find the local ID of this node and append to neighbour frontier.
				if(owner == myRank)
				{
					int lid = gid2lid[nborGID];
					localNeighbours->push_back(lid);
				}
				else
				{
					sendBuf[owner].push_back(nborGID);
				}
			}
		}

		printf("Process %d; My local neighbours size %lu\n", myRank,localNeighbours->size());


		//#pragma omp parallel for private(i)
		//Asking for the adjacency list of the nodes in the sendBuffer.
		MPI_Request request;

		//sending newly visited nbors to their owners in parallel
		for(int i=0; i<localGraph.numParts; i++)
		{
			// Sending the length of the send buffer to the neighbours
			if(sendBuf[i].size())
			{
				MPI_Isend(&(sendBuf[i].front()), sendBuf[i].size(), MPI_INT, i, 1, comm, &request);
				MPI_Request_free(&request);
				printf("%d Sent %d - %d to processor %d\n", myRank, sendBuf[i][0], sendBuf[i][sendBuf[i].size()-1], i);
			}
		}

		//Make this OpenMP.
		for(int i=0; i<localNeighbours->size(); i++){
			std::vector<int> neighbourAdjacency;
			for(int j= localGraph.nborIndex[i]; j<localGraph.nborIndex[i+1]; j++){
				neighbourAdjacency.push_back(localGraph.nborGIDs[j]);
			}
			localTri += intersections(neighbourAdjacency, currentNodeSet);
			printf("------------- Processer %d localTri Updated to %d\n", myRank,localTri);
		}
			

		for(int i=0; i<localGraph.numParts; i++)
		{
			// Rank i gather sendCount[i] from each rank
			// This is the total number of neighbours this node gets
			int s = sendBuf[i].size();
			MPI_Gather(&(s), 1, MPI_INT, &recvCount.front(), 1, MPI_INT, i, comm);
			//printf("Processer %d gathered from %d; will receive size %d\n", myRank,i,recvCount[i]);
			
		}

		for(int i=0; i<localGraph.numParts; i++)
		{
			recvBuf[i] = std::vector<int>(recvCount[i]);
			if(recvCount[i])
			{
				MPI_Recv(&(recvBuf[i].front()), recvCount[i], MPI_INT, i, 1, comm, MPI_STATUS_IGNORE);
				printf("++++++++++++++ Processer %d received %d - %d from processor %d\n", myRank, recvBuf[i][0], recvBuf[i][recvBuf[i].size()-1], i);
			}
		}

		//handling newly visited vertices and compute the distance
		
		for(int i=0; i<localGraph.numParts; i++)
		{
			for(int j=0; j<recvCount[i]; j++)
			{
				int gid = recvBuf[i][j];
				int lid = gid2lid[gid];

				for(int k=localGraph.nborIndex[lid]; k<localGraph.nborIndex[lid+1]; k++){
					sendAdjacency[i].push_back(localGraph.nborGIDs[k]);
				}
				//This is the delimiter
				sendAdjacency[i].push_back(-1);

				//int len = localGraph.nborIndex[lid+1] - localGraph.nborIndex[lid];
				//sendAdjacencyLengths[i].push_back(len);

				printf("############ Process %d sendAdjacency to %d size is %lu\n", myRank,i,sendAdjacency[i].size());
			}

			recvBuf[i].clear(); // is this required?
		}

		// //Send the adjacency list's length of the elements requested.
		// for(int i=0; i<localGraph.numParts; i++)
		// {
		// 	// Sending the length of the send buffer to the neighbours
		// 	if(sendBuf[i].size())
		// 	{
		// 		MPI_Isend(&(sendAdjacencyLengths[i].front()), sendAdjacencyLengths[i].size(), MPI_INT, i, 1, comm, &request);
		// 		MPI_Request_free(&request);
		// 	}
		// }

		//Send the requested elements adjacency list now.
		for(int i=0; i<localGraph.numParts; i++)
		{
			// Sending the length of the send buffer to the neighbours
			if(sendAdjacency[i].size())
			{
				MPI_Isend(&(sendAdjacency[i].front()), sendAdjacency[i].size(), MPI_INT, i, 1, comm, &request);
				MPI_Request_free(&request);
			}
		}

		recvCount.clear();

		for(int i=0; i<localGraph.numParts; i++)
		{
			// Rank i gather sendCount[i] from each rank
			// This is the total number of neighbours this node gets
			int s = sendAdjacency[i].size();
			//MPI_Gather(&(s), 1, MPI_INT, &recvCount.front(), 1, MPI_INT, i, comm);
			MPI_Gather(&(s), 1, MPI_INT, &recvCount.front(), 1, MPI_INT, i, comm);
			
		}

		// //Receive the adjacency list's lengths.
		// for(int i=0; i<localGraph.numParts; i++)
		// {
		// 	recvBuf[i] = std::vector<int>(recvCount[i]);
		// 	if(recvCount[i])
		// 	{
		// 		//The receiving lengths will be of size equal to the requested size
		// 		//If you request n elements, you will get back n element's length
		// 		MPI_Recv(&(recvAdjacencyLengths[i].front()), sendBuf[i].size(), MPI_INT, i, 1, comm, MPI_STATUS_IGNORE);
		// 	}
		// }

		//Receive the adjacency lists of the neigbours requested.
		for(int i=0; i<localGraph.numParts; i++)
		{
			recvAdjacency[i] = std::vector<int>(recvCount[i]);
			if(recvCount[i])
			{
				MPI_Recv(&(recvAdjacency[i].front()), recvCount[i], MPI_INT, i, 1, comm, MPI_STATUS_IGNORE);
			}
		}

		//Outer for loop can use OpenMP directives.
		for(int i=0; i<localGraph.numParts; i++){
			if(recvCount[i]){
				recvAdjacencyLengths[i].push_back(0);
				for(int j=0; j<recvAdjacency[i].size(); j++){
					if(recvAdjacency[i][j] == -1){
						recvAdjacencyLengths[i].push_back(j+1);
					}
				}
			}
		}


		//Check the number of intersections in the data received by the others.
		
		for(int i=0; i<localGraph.numParts; i++){
			
			for(int j=0; j<sendBuf[i].size(); j++){

				std::vector<int> nborAdjacency;
				for(int k=recvAdjacencyLengths[i][j]; k<recvAdjacencyLengths[i][j+1] - 1; k++){
					nborAdjacency.push_back(recvAdjacency[i][k]);
				}
				remoteTri += intersections(nborAdjacency, currentNodeSet);
			}
		}



		//Clear the sendBuf.
		//Clear the receiveBuf. - ALREADY DONE.
		//Clear the adjacency lists received and its lengths ???
		
		for(int i=0; i<localGraph.numParts; i++)
		{
			sendBuf[i].clear();
			recvBuf[i].clear();
		}

		localNeighbours->clear();

		if(iteration < localGraph.numVertices){
			currentNodeGid = localGraph.vertexGIDs[iteration];
			iteration++;
		}else{
			currentNodeGid = -1;
			activeThread = 0;
		}

		MPI_Barrier(comm);
		MPI_Allreduce(MPI_IN_PLACE, &activeThread, 1, MPI_INT, MPI_SUM, comm);

		//TODO: Clear the localNeighbours.
	}

	//TODO: Return the correct number here.
	return localTri+remoteTri;

}

void parseCommandLineArguments(int argc,char *argv[], std::string &ip, std::string &op)
{
	ip = "input/sample_input.txt";
	op = "";
	for(int i = 1; i < argc; i++)
	{
		//cout << (string(argv[i]) == "-o") << endl;
		// if(std::string(argv[i]) == "-r")
		// {
		// 	root = std::stoi(argv[i+1]);
		// 	std::cout << "root set to " << root << std::endl;
		// }
		if(std::string(argv[i]) == "-i")
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

int main(int argc, char *argv[]) {

	int numParts;
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	MPI_Comm_size(comm, &numParts);

	cout << "Number of MPI processes " << numParts << endl;
	
	int srcRank = 0;
	std::string fname, ofname;

	if(myRank == srcRank)
	{
		parseCommandLineArguments(argc,argv,fname,ofname);
	}

	GraphStruct localGraph;

	// TODO: Maybe we should be able to replace this by graphLoad(&localGraph, subGraphFile)

	printf("Load the subgraph\n");
	getSubGraph(comm, &localGraph, fname, numParts);
	srand(time(NULL));

	//Have to call the function to count the triangles
	int localTriangles = CountingTriangles(comm, localGraph, srcRank);

	printf("Process: %d; Local triangles with me are : %d\n", myRank,localTriangles);

	MPI_Allreduce(MPI_IN_PLACE, &localTriangles, 1, MPI_INT, MPI_SUM, comm);

	if(myRank == srcRank){
		printf("Total number of triangles = %d\n", localTriangles);
	}
	//Print the output to a output file.

	//globalDist.clear();
	graphDeinit(&localGraph);
	MPI_Finalize();
	return 0;

}