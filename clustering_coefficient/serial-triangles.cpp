#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

using namespace std;

// #define WRITE_OUTPUT

static clock_t exec_time, read_time;

vector<vector<int> > graph;
vector< unordered_map <int, bool> > opt_graph;

void parseCommandLineArguments(int argc, char *argv[], std::string &ip)
{
	ip = "input/sample_input.txt";
	for(int i = 1; i < argc; i++)
	{
		if(std::string(argv[i]) == "-i")
		{
			ip = std::string(argv[i+1]);
			// std::cout << "input file path " << ip << std::endl;
		}
	}
}

int count_triangles()
{
	int V = graph.size();
	int num_traingles = 0;

	for(int i=0; i<V; i++)
	{
		int num_neighbours = graph[i].size();
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

				if(opt_graph[graph[i][j]].find(graph[i][k]) != opt_graph[graph[i][j]].end())
				{
					num_traingles += 1;
				}
			}
		}
	}
	return num_traingles;
}

int main(int argc, char *argv[])
{
	vector<int> temp;
	unordered_map <int, bool> opt_temp;

	std::string fname, ofname;
	int ignore;
	parseCommandLineArguments(argc, argv, fname);
	FILE *fp = fopen(fname.c_str(), "r");

	int V, E;
	// Number of vertices, edges
	fscanf(fp, "%d%d", &V, &E);
	
	// Number of partitions, ignore this.
	fscanf(fp, "%d", &ignore);

	// Take the input.
	clock_t start = clock();
	for(int i=0; i<V; i++)
	{
		graph.push_back(temp);
		opt_graph.push_back(opt_temp);
	}
	for(int i=0; i<V; i++)
	{
		int vid, num_neighbours;
		fscanf(fp, "%d%d%d", &vid, &ignore, &num_neighbours);
		for(int j=0; j<num_neighbours; j++)
		{
			int neighbourID;
			fscanf(fp, "%d", &neighbourID);
		
			// if(neighbourID != i)
			// {		
			// 	graph[i].push_back(neighbourID);
			// 	opt_graph[i][neighbourID] = true;
			// }

			if(i < neighbourID)
			{		
				graph[i].push_back(neighbourID);
				opt_graph[i][neighbourID] = true;

				graph[neighbourID].push_back(i);
				opt_graph[neighbourID][i] = true;
			}
		}
	}
	clock_t stop = clock();
	read_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

	start = clock();
	int num_traingles = count_triangles();
	stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

	
	printf("Num traiangles %d\n", num_traingles);
	cout << "Reading time " << read_time << "ms" << endl;
	cout << "CC time " << exec_time << "ms" << endl;
	return 0;
}