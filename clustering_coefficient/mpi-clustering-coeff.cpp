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

#define WRITE_OUTPUT

static clock_t exec_time, read_time;
static int myRank;

unordered_map<int, vector<int> > graph;
unordered_map<int, int> isOwner;
unordered_map<int, unordered_map <int, bool> > opt_graph;
unordered_map<int, double> clustering_coefficient;
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

void compute_clustering_coefficient(MPI_Comm comm)
{
	int V = GIDs.size();

	for(int v=0; v<V; v++)
	{
		int i = GIDs[v];
		int num_neighbours = graph[i].size();
		if(isOwner[i] == 0)
			continue;
		if(num_neighbours <= 1)
			continue;

		double neu = 0.0;

		for(int j=0; j < num_neighbours; j++)
		{
			for(int k=0; k < num_neighbours; k++)
			{
				// if graph[i][j] is a neighbour of graph[i][k], then add 2 to neu.
				if(opt_graph[graph[i][j]].find(graph[i][k]) != opt_graph[graph[i][j]].end())
				{
					neu += 1;
				}
			}
		}
		if(num_neighbours > 1)
		{
			double den = ((double)(num_neighbours)) * ((double)(num_neighbours - 1));

			clustering_coefficient[i] = neu/den;
		}
	}
	MPI_Barrier(comm);
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
		
		clustering_coefficient[vid] = 0.0;
		GIDs.push_back(vid);

		graph[vid] = vector<int>();
		opt_graph[vid] = unordered_map <int, bool>();
		
		for(int j=0; j<num_neighbours; j++)
		{
			int neighbourID;
			fscanf(fp, "%d", &neighbourID);
			if(neighbourID != vid)
			{
				graph[vid].push_back(neighbourID);

				opt_graph[vid][neighbourID] = true;
			}
		}
	}
	clock_t stop = clock();
	read_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

	start = clock();
	compute_clustering_coefficient(comm);
	stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

#ifdef WRITE_OUTPUT
	for(int r = 0; r < numParts; r++)
	{
		if(r != myRank)
			continue;
		for(int i=0; i<V; i++)
		{
			if(isOwner[GIDs[i]] == 0)
				continue;
			printf("%d %.6lf\n", GIDs[i], clustering_coefficient[GIDs[i]]);
		}
	}
#endif

	// cout << "Reading time " << read_time << "ms" << endl;
	// cout << "CC time " << exec_time << "ms" << endl;
	// cout << "Rank " << myRank << "  " << num_local << endl;
	MPI_Finalize();
	return 0;
}