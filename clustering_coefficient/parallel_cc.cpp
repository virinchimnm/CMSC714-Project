#include <time.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <mpi.h>
using namespace std;

 //#define WRITE_OUTPUT


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



void print_vector(vector<long long int> vec){
    for(int i=0;i<vec.size();i++){
        cout<<vec[i]<<" ";
    }
    cout<<endl;
}


void read_file(const char* filename, int rank, vector<long long int>& local_nodes, MAP& local_adjlist){

    string str;
    long long int val;
    string outfilename = filename;
    outfilename.append(".");
    outfilename.append(std::to_string(rank));

    ifstream file(outfilename);
    
    int i=0,j;

    while (getline(file, str)){
        
        stringstream ss(str);
        
        
        // if reading first line
        if(i==0){
            while(ss >> val){
                local_nodes.push_back(val);
            }
        }else{
            j=0;
            long long int src,dest;
            while(ss >> val){

                if(j==0){
                    src = val;
                }else{
                    dest=val;
                    local_adjlist[src][dest]=true;
                }
                j++;
            }

        }
        i++;
        

    }

    return;
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
#ifdef WRITE_OUTPUT
        cout<<local_nodes[i]<<","<<cc<<endl;
#endif
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
    vector<long long int> local_nodes;
    
     
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

    
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD; 


	MPI_Comm_rank(comm, &rank); 
	MPI_Comm_size(comm, &world_size); 

	

    // read from corresponding file the local nodes and local graph

    read_file(infile,rank,local_nodes,local_adjlist);

    clock_t start = clock();
    compute_cc(local_adjlist,local_nodes);
    clock_t stop = clock();
    double exec_time = double(stop - start) / (CLOCKS_PER_SEC / 1000.00);
    cout << " exec_time " << exec_time << "ms" << endl;
    


    MPI_Barrier(comm);
    MPI_Finalize();

    return 0;
}
