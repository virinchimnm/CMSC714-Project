#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <mpi.h>

using namespace std;

// #define WRITE_OUTPUT

static clock_t exec_time, read_time;
static int myRank;

unordered_map<int, vector<int> > graph;
unordered_map<int, int> isOwner;
unordered_map<int, unordered_map <int, bool> > opt_graph;
vector<int> GIDs;

void parseCommandLineArguments(int argc, char *argv[], std::string &ip)
{
	ip = "input/sample_input.txt";
	string part = to_string(myRank);
	for(int i = 1; i < argc; i++)
	{
		if(std::string(argv[i]) == "-i")
		{
			ip = std::string(argv[i+1]) + "." + part;
			// std::cout << "input file path " << ip << std::endl;
		}
	}
}

int count_triangles(MPI_Comm comm)
{
	int V = GIDs.size();
	int num_traingles = 0;

	for(int v=0; v<V; v++)
	{
		int i = GIDs[v];
		int num_neighbours = graph[i].size();
		if(isOwner[i] == 0)
			continue;
		if(num_neighbours <= 1)
			continue;

		for(int j=0; j < num_neighbours; j++)
		{
			if(graph[i][j] < i)
				continue;
			for(int k=0; k < num_neighbours; k++)
			{
				if(graph[i][k] < graph[i][j])
					continue;

				// if graph[i][j] is a neighbour of graph[i][k], then add 1 to num_neighbours.
				if(opt_graph[graph[i][j]].find(graph[i][k]) != opt_graph[graph[i][j]].end())
				{
					num_traingles += 1;
				}
			}
		}
		
	}
	MPI_Barrier(comm);
	return num_traingles;
}

int main(int argc, char *argv[])
{
	int numParts;
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	MPI_Comm_size(comm, &numParts);

	std::string fname, ofname;
	int ignore;
	parseCommandLineArguments(argc, argv, fname);
	FILE *fp = fopen(fname.c_str(), "r");

	int V;
	// Number of vertices, edges
	fscanf(fp, "%d", &V);
	
	// Take the input.
	
	// vector<int> temp;
	// unordered_map <int, bool> opt_temp;
	int num_local = 0;

	clock_t start = clock();
	for(int i=0; i<V; i++)
	{
		int vid, num_neighbours, owner;
		fscanf(fp, "%d%d%d", &vid, &owner, &num_neighbours);
		isOwner[vid] = owner;
		if(owner == 1)
			num_local++;
		
		GIDs.push_back(vid);

		graph[vid] = vector<int>();
		opt_graph[vid] = unordered_map <int, bool>();
		
		for(int j=0; j<num_neighbours; j++)
		{
			int neighbourID;
			fscanf(fp, "%d", &neighbourID);
			// if(neighbourID != vid)
			// {
			// 	graph[vid].push_back(neighbourID);

			// 	opt_graph[vid][neighbourID] = true;
			// }

			// By default treat it as undirected
			if(vid < neighbourID)
			{		
				graph[vid].push_back(neighbourID);
				opt_graph[vid][neighbourID] = true;

				graph[neighbourID].push_back(vid);
				opt_graph[neighbourID][vid] = true;
			}
		}
	}
	clock_t stop = clock();
	read_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

	start = clock();
	int num_triangles = count_triangles(comm);
	stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

	int total_triangles;
	MPI_Reduce(&num_triangles, &total_triangles, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if(myRank == 0)
	{
		printf("Total Triangles = %d\n", total_triangles);
	}

	cout << "Reading time " << read_time << "ms" << endl;
	cout << "Exec time " << exec_time << "ms" << endl;
	MPI_Finalize();
	return 0;
}