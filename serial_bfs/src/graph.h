#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>
#include <queue>
#include <fstream>
#include <ctime>

class Graph
{
public:	
		int N_v; //no. of vertices
		int N_e;
		std::vector<std::vector<int>> adjList;
		std::vector<std::vector<int>> bfs_tree;
		std::vector<int> dist;
		double exec_time;

		Graph(){};
		Graph(int n_v, int n_e); //pass no. of vertices

		void addEdge(int u, int v); //add edge from u to v
		
		std::vector<int>& getNeighbors(int u); //get all neighbors of vertex u
		void bfs(int root);
		void dispGraph();
		void dispBFSTree();
		void saveBFSTree(std::string &op);
};

Graph::Graph(int n_v, int n_e): adjList(n_v)
{
	N_e = n_e;
	N_v = n_v;
}

void Graph::addEdge(int u, int v)
{
	adjList[u].push_back(v);
}

std::vector<int>& Graph::getNeighbors(int u)
{
	return adjList[u];
}

void Graph::dispGraph()
{
	for(int i = 0; i < N_v; i++)
	{
		std::cout<< "Node: " << i << ":: ";
		std::vector<int> neighbors = getNeighbors(i);
		for(const int n : neighbors)
			std::cout << n << " ";
		std::cout << std::endl;
	}
}

void Graph::dispBFSTree()
{
	// int lvl_cnt = 0;
	// for(const std::vector<int> lvl : bfs_tree)
	// {
	// 	std::cout << "L[" << lvl_cnt++ << "]: ";
	// 	for(const int node: lvl)
	// 		std::cout << node << " ";
	// 	std::cout << std::endl;
	// }
	std::cout << "exec_time : " <<exec_time << "ms" << std::endl;

	for(int i=0; i<N_v; i++)
		std::cout << "d[" << i << "] = " << dist[i] << std::endl;
}

void Graph::saveBFSTree(std::string &op)
{
	std::ofstream myfile;
	myfile.open (op, std::ofstream::out | std::ofstream::trunc);

	for(const std::vector<int> lvl : bfs_tree)
	{
		for(const int node: lvl)
			myfile << node << " ";
		myfile << std::endl;
	}
	myfile << exec_time <<"ms"<<std::endl;
	myfile.close();	
	std::cout << "Output Save to " << op << std::endl;						
}


void Graph::bfs(int root)
{
	int lvl = 0;
	std::vector<std::queue<int>> queue(2, std::queue<int>());
	std::queue<int> *curr_ptr = &queue[0], *next_ptr = &queue[1], *tmp;
	std::vector<bool> visited(N_v,0);

	dist.clear();
	for(int i=0; i<N_v; i++)
		dist.push_back(-1);

	clock_t start = clock();
	curr_ptr -> push(root);
	visited[root] = true;
	dist[root] = 0;
	while(!curr_ptr -> empty())
	{
		//std::cout << "level :" << lvl << ":" << curr_ptr->size()  << std::endl;
		bfs_tree.push_back(std::vector<int>());
		while(!curr_ptr->empty())
		{
			int node = curr_ptr -> front();
			bfs_tree[lvl].push_back(node);
			
			//std::cout << " " << node ; 
			for(const int n : adjList[node])
			{
				if(!visited[n])
				{
					next_ptr->push(n);
					visited[n] = true;
					dist[n] = lvl+1;
				}
			}

			curr_ptr->pop();
		}
		//std::cout << std::endl;
		tmp = curr_ptr;
		curr_ptr = next_ptr;
		next_ptr = tmp;
		lvl++;
	}
	clock_t stop = clock();
	exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);
}

#endif