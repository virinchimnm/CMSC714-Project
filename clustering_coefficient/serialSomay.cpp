#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

using namespace std;

#define WRITE_OUTPUT

static double d = 0.85;
static clock_t exec_time;

vector<vector<int> > graph;
vector< unordered_map <int, bool> > opt_graph;
vector<double> clustering_coefficient;

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

void compute_clustering_coefficient()
{
	int V = graph.size();

	for(int i=0; i<V; i++)
	{
		int num_neighbours = graph[i].size();
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
	for(int i=0; i<V; i++)
	{
		graph.push_back(temp);
		opt_graph.push_back(opt_temp);
		clustering_coefficient.push_back(0.0);
	}
	for(int i=0; i<V; i++)
	{
		int vid, num_neighbours;
		fscanf(fp, "%d%d%d", &vid, &ignore, &num_neighbours);
		for(int j=0; j<num_neighbours; j++)
		{
			int neighbourID;
			fscanf(fp, "%d", &neighbourID);
		
			if(neighbourID != i)
			{		
				graph[i].push_back(neighbourID);
				opt_graph[i][neighbourID] = true;
			}
		}
	}

	compute_clustering_coefficient();

	double overall = 0;
	for(int i=0; i<V; i++)
	{
		printf("%d %.6lf\n", i, clustering_coefficient[i]);
		overall += clustering_coefficient[i];
	}	
	overall = overall/V;
	printf("%.6lf\n", overall);
	return 0;
}