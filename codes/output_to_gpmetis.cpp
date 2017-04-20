#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <unordered_map>
#include <vector>
// #include <metis.h>
using namespace std;


void read_graph(const char* filename){

    ifstream infile;
    long long int a,b,e=0,i=1;

    unordered_map<long long int, unordered_map<long long int, bool> > adjlist;

    infile.open(filename,ios::in);

    if(infile.is_open())
    {
        while(infile>>a>>b)
        {
            if(a == b)
            {
                cerr << "Found a self edge" << endl;
                continue;
            }
            if(adjlist[a].find(b) == adjlist[a].end())
            {
                adjlist[a][b] = true;
                e++;
            }
        }
    }

    infile.close();

    cout<<adjlist.size()<<" "<<e/2<<endl;

    unordered_map<long long int, std::unordered_map<long long int, bool> >::iterator src_it;

    for(int i=0;i<adjlist.size();i++){

      unordered_map<long long int, bool>::iterator dest_it;
      int num_neighbours = 0;

      // indexing for gpmetis starts from 1--- list the adjacent nodes
      for(dest_it=adjlist[i].begin();dest_it!=adjlist[i].end();++dest_it){
        cout<< dest_it->first+1 << " ";
        num_neighbours++;
      }
      cout<<endl;

      if(num_neighbours == 0)
      {
        cerr << "Vertex " << i << " has no neighbours" << endl;
      }
    }
}


int main(int argc, char* argv[]){

    char ch;
    const char* infile;

    while( (ch = getopt( argc, argv, "g:n:")) != -1 )
    {
        if(ch == 'g'){
            infile = optarg;
        }
    }

    if(argc < optind)
    {
        cout<< "./output_to_gpmetis -g path-to-graph\n";
        exit(0);
    }

    read_graph(infile);

    return 0;
}
