#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
// #include <metis.h>
using namespace std;


void read_graph(const char* filename1, const char* filename2){

    ifstream infile;
    long long int a,b,i, e = 0;


    unordered_map<long long int, long long int> part_map;
    unordered_map<long long int, unordered_map<long long int, bool> > adjlist;

    // read scaled graph file

    infile.open(filename1,ios::in);

    if(infile.is_open())
    {
        while(infile>>a>>b)
        {
            if(a != b){
              adjlist[a][b] = true;
              e++;

            }
        }
    }
    infile.close();



    //read partition file created by metis

    infile.open(filename2,ios::in);

    long long int num_part = 0;
    if(infile.is_open())
    {
        i=0;
        while(infile>>a)
        {
            part_map[i++] = a;
            num_part = max(num_part, a);
        }
    }
    infile.close();


    cout << adjlist.size() << endl;
    cout << e/2 << endl;
    cout << num_part + 1 << endl;

    unordered_map<long long int, std::unordered_map<long long int, bool> >::iterator src_it;

    for(int i=0;i<adjlist.size();i++){

      unordered_map<long long int, bool>::iterator dest_it;

      cout<<i<<" "<<part_map[i]<<" "<<adjlist[i].size()<<" ";

      // indexing for gpmetis starts from 1--- list the adjacent nodes
      for(dest_it=adjlist[i].begin();dest_it!=adjlist[i].end();++dest_it){
        cout<<dest_it->first+1<<" ";
      }
      cout<<endl;
    }
}


int main(int argc, char* argv[]){

    char ch;
    const char* infile;
    const char* parfile;

    while( (ch = getopt( argc, argv, "g:p:")) != -1 )
    {
        if(ch == 'g'){
            infile = optarg;
        }
        if(ch == 'p'){
            parfile = optarg;
        }
    }

    if(argc < optind)
    {
        cout<< "./output_to_graph_algo -g path-to-scaled-graph -p path-partition-file\n";
        exit(0);
    }

    read_graph(infile,parfile);

    return 0;
}
