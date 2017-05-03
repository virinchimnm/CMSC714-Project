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

void parseCommandLineArguments(int argc,char *argv[], std::string &ip, std::string &op, int &num_iter)
{
	ip = "input/sample_input.txt";
	op = "";
	for(int i = 1; i < argc; i++)
	{
		if(std::string(argv[i]) == "-i")
		{
			ip = std::string(argv[i+1]);
			// std::cout << "input file path " << ip << std::endl;
		}
		else if(std::string(argv[i]) == "-iter")
		{
			num_iter = std::stoi(argv[i+1]);
			// std::cout << "Number of iterations " << num_iter << std::endl;
		}
	}
}

int main(int argc, char *argv[])
{
	vector<vector<int> > graph;
	vector<int> temp;
	vector<int> degree;

	std::string fname, ofname;
	int num_iter = 10, ignore;
	parseCommandLineArguments(argc, argv, fname, ofname, num_iter);
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
		int vid, num_neighbours;
		fscanf(fp, "%d%d%d", &vid, &ignore, &num_neighbours);
		for(int j=0; j<num_neighbours; j++)
		{
			int neighbourID;
			fscanf(fp, "%d", &neighbourID);
			graph[i].push_back(neighbourID);
		}	
		degree.push_back(num_neighbours);	
	}

	double *pageRank = (double*) malloc(sizeof(double) * 2 * V);
	double *oldPageRank = pageRank;
	double *currPageRank = pageRank + V;
	double *swap_temp;

	clock_t start = clock();

	for(int i=0; i<V; i++)
	{
		oldPageRank[i] = 1.0/ ((double)V);
	}

	for(int iter=0; iter < num_iter; iter++)
	{
		for(int i=0; i<V; i++)
		{
			currPageRank[i] = (1 - d) / ((double) V);

			int num_neighbours = graph[i].size();
			for(int j=0; j<num_neighbours; j++)
			{
				int neighbour = graph[i][j];
				currPageRank[i] += d * (oldPageRank[neighbour] / ((double) degree[neighbour]) );
			}
		}

		swap_temp = currPageRank;
		currPageRank = oldPageRank;
		oldPageRank = swap_temp;
	}

	clock_t stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);

#ifdef WRITE_OUTPUT
	for(int i=0; i<V; i++)
	{
		printf("%d %.8lf\n", i, oldPageRank[i]);
	}
#endif

	cout << exec_time <<"ms (serial)"<< endl;

	return 0;
}