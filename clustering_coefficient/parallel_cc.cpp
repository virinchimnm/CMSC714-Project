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

int SIZE_TAG = 1, LOCAL_TAG = 2;

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


void print_vector(vector<long long int> vec){

	for(int i=0;i<vec.size();i++){
		cout<<vec[i]<<" ";
	}
	cout<<endl;
}

// send local graph to processor i
void send_graph(MPI_Comm comm, MAP local_adjlist, vector<long long int> local_nodes, int i){

	//sending local nodes to processor
	long long int local_size = local_nodes.size();
	MPI_Send(&local_size,1, MPI_LONG_LONG, i, SIZE_TAG, comm);
	MPI_Send(&local_nodes[0], local_size, MPI_LONG_LONG, i, LOCAL_TAG, comm);

	//send number of adjacency lists
	long long int num = local_adjlist.size();
	MPI_Send(&num,1, MPI_LONG_LONG, i, SIZE_TAG, comm);
	
	
	
	MAP::iterator src_it;
    for(src_it=local_adjlist.begin();src_it!=local_adjlist.end();++src_it){
        unordered_map<long long int, bool>::iterator dest_it;
        vector<long long int> adj;
        adj.push_back(src_it->first);

        for(dest_it = src_it->second.begin();dest_it != src_it->second.end();++dest_it){
            adj.push_back(dest_it->first);   
        }
        
        
        //send size of each adjacency list
        num = adj.size();
        MPI_Send(&num,1, MPI_LONG_LONG, i, SIZE_TAG, comm);

        //send each adjacency list
        MPI_Send(&adj[0], num, MPI_LONG_LONG, i, LOCAL_TAG, comm);
		
    }

	return;
}


MAP receive_graph(MPI_Comm comm, vector<long long int>& local_nodes){

	MAP local_adjlist;
	long long int local_size,num;
    
	// Recvng local nodes from processor
	MPI_Recv(&local_size,1, MPI_LONG_LONG,0,SIZE_TAG,comm,MPI_STATUS_IGNORE);
	local_nodes.resize(local_size);
	MPI_Recv(&local_nodes[0], local_size, MPI_LONG_LONG, 0, LOCAL_TAG, comm,MPI_STATUS_IGNORE);

	//Recvng number of adjacency list 
	MPI_Recv(&num,1, MPI_LONG_LONG,0,SIZE_TAG,comm,MPI_STATUS_IGNORE);

	for(int i=0;i<num;i++){

		//recv size of adjacency list to rcv
		long long int n;
		MPI_Recv(&n,1, MPI_LONG_LONG,0,SIZE_TAG,comm,MPI_STATUS_IGNORE);
		
		// recv the adjacency list 
		vector<long long int> adj_list(n);
		MPI_Recv(&adj_list[0], n, MPI_LONG_LONG, 0, LOCAL_TAG, comm,MPI_STATUS_IGNORE);

		//populate the local_graph
		long long int src = adj_list[0],dest;

		for(int j=1;j<adj_list.size();j++){
			dest = adj_list[j];
			local_adjlist[src][dest]=true;
		}
	}


	
	return local_adjlist;
}


//creating and distributing local graphs 
// only rank 0 process does this
MAP distribute_local_graphs(MPI_Comm comm, MAP global_adjlist, unordered_map <long long int, int> node_par_map, unordered_map <int, vector<long long int> > par_node_map, vector<long long int>& lnodes ){

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
		// do sends here
		if (i != 0){
			send_graph(comm,local_adjlist,local_nodes,i);
		}else{
			lnodes = local_nodes;
			return local_adjlist;
		}

	}
}



void compute_cc(MAP adjlist, vector<long long int> local_nodes){

    long long int i=0,j,k;
    int num,den;
    unordered_map<long long int, bool>::iterator dest_it;
    double cc,gcc=0;
    vector<long long int> vec;

    for(i=0;i<local_nodes.size();i++){

        //cout<<local_nodes[i]<<endl;
        for(dest_it=adjlist[local_nodes[i]].begin();dest_it!=adjlist[local_nodes[i]].end();++dest_it){
        	if(local_nodes[i]!=dest_it->first){
        		vec.push_back(dest_it->first);
        	}
            
        }

        num=0;
        den = vec.size();

        for(j=0;j<vec.size();j++){
            for(k=j+1;k<vec.size();k++){
                if(adjlist[vec[j]][vec[k]]){
                    num++;
                }
            }
        }

        if(den==0 || den==1){
            cc=0;
        }else{
            cc=2.0*num/(den*(den-1));
        }
        
        //gcc+=cc;
        cout<<local_nodes[i]<<","<<cc<<endl;
        vec.clear();
        
    }

    //cout<<"Global CC= "<<1.0*gcc/adjlist.size()<<endl;

    return;
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

	vector<long long int> local_nodes;

    if(rank == 0){
    	//generate local_graphs and send it
    	MAP global_adjlist;
    	unordered_map <long long int, int> node_par_map;
    	unordered_map <int, vector<long long int> > par_node_map;
    	init_global_graph(infile,comm,global_adjlist,node_par_map,par_node_map); 
    	local_adjlist = distribute_local_graphs(comm,global_adjlist,node_par_map,par_node_map,local_nodes);
    }else{
    	//receive the subgraph
    	local_adjlist = receive_graph(comm,local_nodes);
    }
    
    compute_cc(local_adjlist,local_nodes);

    MPI_Finalize();

    return 0;
}
