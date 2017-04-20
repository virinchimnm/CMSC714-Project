#include "graph.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <limits>

using namespace std;
void disp(vector<string> &s);
vector<string> parseIn();


void disp(vector<string> &s)
{
	for(auto i : s)
		cout <<  i << endl;
}

vector<string> parseIn()
{
	cout << "parsing input file" << endl;
	string s;
	vector<string> lines;
	while(getline(cin,s))
	{
		if(s[0] != '#')
		{
			lines.push_back(s);
		}
	}
	//disp(lines);
	cout << "..... Done!" << endl;
	return lines;
}

void initGraph(Graph &g, vector<string> &lines)
{
	int line_cnt = 0;
	int n_v = stoi(lines[line_cnt++]);
	int n_e = stoi(lines[line_cnt++]);
	g =  Graph(n_v,n_e);
	cout << "n_v = " << n_v << " n_e " << n_e<<endl;
	int n_p = stoi(lines[line_cnt++]);
	// cout << "skipping :" << n_p << " partition info lines" << endl;
	// line_cnt+=n_p;

	cout << "First Edge Info Line:" << endl;
	cout << lines[line_cnt] << endl;

	for(; line_cnt < lines.size(); line_cnt++)
	{
		int u, num_edges, p;
		stringstream ss(lines[line_cnt]);
		ss >> u >> p >> num_edges;
		for(int i = 0; i < num_edges; i++)
		{
			int v;
			ss >> v;
			g.addEdge(u,v);
		}
	}

	cout << "Last Edge Info Line:" << endl;
	cout << lines[line_cnt-1] << endl;
	cout << ".....Graph Initialzed!! " << endl;
}

void parseCommandLineArguments(int argc,char *argv[], int &root, string &op)
{
	root = 0;
	for(int i = 1; i < argc; i++)
	{
		//cout << (string(argv[i]) == "-o") << endl;
		if(string(argv[i]) == "-r")
		{
			root = stoi(argv[i+1]);
			cout << "root set to " << root << endl;
		}
		else if(string(argv[i]) == "-o")
		{
			op = string(argv[i+1]);
			cout << "output file path " << op << endl;
		}
	}
}

int main(int argc, char *argv[])
{
	int root;
	string op = "";
	parseCommandLineArguments(argc,argv,root, op);
	Graph g;
	vector<string> lines = parseIn();
	initGraph(g,lines);
	cout << "Graph with " << g.N_v << " created" << endl;
	g.bfs(root);
	g.dispBFSTree();
	if(op != "")
	{
		g.saveBFSTree(op);
	}
	return  0;
}