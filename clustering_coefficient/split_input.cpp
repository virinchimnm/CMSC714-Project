#include <time.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;


typedef unordered_map<long long int, unordered_map<long long int, bool> > MAP;


void write_graph(string filename, vector<long long int> local_nodes, MAP adjlist){
    
    ofstream ofile;
    ofile.open (filename,ios::out);

    ofile<<adjlist.size()<<endl;

    MAP::iterator src_it;
    for(src_it=adjlist.begin();src_it!=adjlist.end();++src_it)
    {

        unordered_map<long long int, bool>::iterator dest_it;
        vector<long long int> vec;
        
        for(dest_it = src_it->second.begin();dest_it != src_it->second.end();++dest_it)
        {
            vec.push_back(dest_it->first);
            //ofile<<dest_it->first<<" ";   
        }

        ofile<<src_it->first<<" ";

        //check if this node is local or not
        
        if(std::find(local_nodes.begin(), local_nodes.end(), src_it->first) != local_nodes.end()){
            ofile<<1<<" ";
        }else{
            ofile<<0<<" ";
        }

        ofile<<vec.size()<<" ";

        for(int i=0;i<vec.size();i++){
            ofile<<vec[i]<<" ";
        }

        ofile<<endl;
    }

    ofile.close();
    return;
}


void init_global_graph(const char* filename, MAP& global_adjlist, unordered_map <long long int, int>& node_par_map, unordered_map <int, vector<long long int> >& par_node_map){

    //read file and create global adjlist
    
    FILE* fp = fopen(filename,"r");

    if (fp == NULL) {
        perror("File Opening Error");
        exit(0);
    } 

    long long int i,j, numGlobalVertices, numGlobalEdges, node;
    int numParts;
   
    fscanf(fp, "%lld", &numGlobalVertices);
    fscanf(fp, "%lld", &numGlobalEdges);
    fscanf(fp, "%d", &numParts);

    //cout<<numGlobalVertices<<" "<<numGlobalEdges<<" "<<numParts<<endl;

    //cout<<world_size<<endl;

    for(i=0;i<numGlobalVertices;i++){
        
        long long int j1,n;
        int p;
        
        fscanf(fp, "%lld%d%lld", &j1, &p, &n);

        node_par_map[i]=p;

        if(global_adjlist.find(p)==global_adjlist.end()){
            vector<long long int> u;
            u.push_back(i);
            par_node_map[p]=u;
        }else{
            vector<long long int> u = par_node_map[p];
            u.push_back(i);
            par_node_map[p]=u;
        }


        for (j = 0; j< n; j++) {
            fscanf(fp, "%lld", &node);
            global_adjlist[i][node] = true;
        }
    }

    fclose(fp);
 
    return;
}


void write_local_graphs(const char* filename,MAP global_adjlist, unordered_map <long long int, int> node_par_map, unordered_map <int, vector<long long int> > par_node_map){

    int i,j,par=par_node_map.size();
    

    //creating subgraph for each processor
    for(i=par-1;i>=0;i--){
    
        //cout<<"Processor "<<i<<endl;
        MAP local_adjlist;

        vector<long long int> local_nodes = par_node_map[i];
        vector<long long int> remote_nodes;
        long long int local_size = local_nodes.size(),src,dest;

        //creating the subgraph
        //might add remote nodes as nodes
        for(j=0; j<local_size;j++){
            src = local_nodes[j]; 

            unordered_map<long long int, bool>::iterator dest_it;
            for(dest_it = global_adjlist[src].begin();dest_it != global_adjlist[src].end();++dest_it)
            {
                dest = dest_it->first;
                local_adjlist[src][dest] = true;

                if(node_par_map[dest] != i){
                    remote_nodes.push_back(dest);
                }
            } 
        }


        long long int remote_size = remote_nodes.size();
        // add edges from remote to local node
        for(j=0; j<remote_size;j++){
            src = remote_nodes[j];
            
            unordered_map<long long int, bool>::iterator dest_it;
            for(dest_it = global_adjlist[src].begin();dest_it != global_adjlist[src].end();++dest_it)
            {
                dest = dest_it->first;
                
                if(node_par_map[dest]==i){
                    local_adjlist[src][dest] = true;
                }  
            }
        }

        // add edges between remote nodes
        for(j=0; j<remote_size;j++){
            src = remote_nodes[j];
            
            unordered_map<long long int, bool>::iterator dest_it;
            for(dest_it = global_adjlist[src].begin();dest_it != global_adjlist[src].end();++dest_it)
            {
                dest = dest_it->first;

                // remote nodes are already added in map
                if(local_adjlist.find(dest)!=local_adjlist.end()){
                    local_adjlist[src][dest] = true;
                }
            }
        }
        
        //local subgraph created
        //write to file here

        string outfilename = filename;
        outfilename.append(".");
        outfilename.append(std::to_string(i));

        write_graph(outfilename, local_nodes, local_adjlist);

    }

    return;
}


int main(int argc, char* argv[]){

    char ch;
    const char* infile;
    int world_size;
    MAP local_adjlist;
    
     
    while( (ch = getopt( argc, argv, "g:n:")) != -1 )
    {
        if(ch == 'g'){
            infile = optarg;
        }
    }

    if(argc < optind)
    {
        cout<< "./output_to_gpmetis -g path-to-graph -n no-of-cores\n";
        exit(0);
    }


    // number of parts is figured out from input file
    
    MAP global_adjlist;
    unordered_map <long long int, int> node_par_map;
    unordered_map <int, vector<long long int> > par_node_map;
    
    // first line has list of local nodes
    // second line onwards ---> graph

    //populate global_adjlist, node_par_map,par_node_map
    init_global_graph(infile,global_adjlist,node_par_map,par_node_map); 

    //write local graph
    write_local_graphs(infile,global_adjlist,node_par_map,par_node_map);
	


    return 0;
}
