#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <mpi.h>
using namespace std;


typedef unordered_map<long long int, unordered_map<long long int, bool> > MAP;

void disp_graph(MAP adjlist){
 
    MAP::iterator src_it;

    for(src_it=adjlist.begin();src_it!=adjlist.end();++src_it)
    {

        unordered_map<long long int, bool>::iterator dest_it;

        cout<<src_it->first<<" : ";

        for(dest_it = src_it->second.begin();dest_it != src_it->second.end();++dest_it)
        {
            cout<<dest_it->first<<" ";   
        }
        cout<<endl;

    }

    return;
}

void init_global_graph(const char* filename, MPI_Comm comm, MAP& global_adjlist, unordered_map <long long int, int>& node_par_map, unordered_map <int, vector<long long int> >& par_node_map){

	//read file and create global adjlist
	
	int world_size;
	MPI_Comm_size(comm, &world_size);
		
	FILE* fp = fopen(filename,"r");

	if (fp == NULL) {
  		perror("File Opening Error");
  		MPI_Finalize();
  		exit(0);
	} 

    long long int i,j, numGlobalVertices, numGlobalEdges, node;
    int numParts;
   
    fscanf(fp, "%lld", &numGlobalVertices);
    fscanf(fp, "%lld", &numGlobalEdges);
    fscanf(fp, "%d", &numParts);

    if(world_size != numParts){
    	cout<<"\nDataset split and Runtime parameters do not match\n";
    	MPI_Finalize();
    	fclose(fp);
    	exit(0);
    }

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

//creating and distributing local graphs 
// only rank 0 process does this
MAP distribute_local_graphs(MAP global_adjlist, unordered_map <long long int, int> node_par_map, unordered_map <int, vector<long long int> > par_node_map){

	int i,j,par=par_node_map.size();
	MAP local_adjlist;

	//creating subgraph for each processor
	for(i=0;i<par;i++){
	
		cout<<"Processor "<<i<<endl;

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
		// do sends here


		local_adjlist.clear();
	}

	

	return local_adjlist;
}


int main(int argc, char* argv[]){

    char ch;
    const char* infile;
    int rank,world_size;
    MAP local_adjlist;
    
     
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

    
    MPI_Init(NULL, NULL);
    MPI_Comm comm = MPI_COMM_WORLD; 

	MPI_Comm_rank(comm, &rank); 
	MPI_Comm_size(comm, &world_size); 


    if(rank == 0){
    	MAP global_adjlist;
    	unordered_map <long long int, int> node_par_map;
    	unordered_map <int, vector<long long int> > par_node_map;
    	init_global_graph(infile,comm,global_adjlist,node_par_map,par_node_map); 

    	/*for(int j=0;j<world_size;j++){

    		vector<long long int> u = par_node_map[j];
    		cout<<"Processor "<<j<<":";
    		for(int i=0;i<u.size();i++){
    			cout<<u[i]<<" ";
    		}
    		cout<<endl;
    	}*/

    	local_adjlist = distribute_local_graphs(global_adjlist,node_par_map,par_node_map);
    }else{
    	//receive the subgraph

    }
    
    //compute_cc(adjlist);

    MPI_Finalize();

    return 0;
}
