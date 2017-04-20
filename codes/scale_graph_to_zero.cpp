#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <metis.h>
using namespace std;


void read_graph(const char* filename){

    ifstream infile;
    long long int a,b,i=0;

    unordered_map<long long int, long long int> node_map;

    infile.open(filename,ios::in);

    if(infile.is_open())
    {
        while(infile>>a>>b)
        {

            if(node_map.find(a)==node_map.end())
            {
                node_map[a] = i++;
            }

            if(node_map.find(b)==node_map.end())
            {
                node_map[b] = i++;
            }

            cout<<node_map[a]<<" "<<node_map[b]<<endl;
        }

    }
    infile.close();
}


int main(int argc, char* argv[]){

    char ch;
    const char* infile;

    while( (ch = getopt( argc, argv, "g:")) != -1 )
    {
        if(ch == 'g'){
            infile = optarg;
        }
    }

    if(argc < optind)
    {
        cout<< "./scale_graph_to_zero -g path-to-graph\n";
        exit(0);
    }

    read_graph(infile);

    return 0;
}
