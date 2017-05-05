#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>
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

MAP init_graph(const char* filename){

    FILE* fp = fopen(filename,"r");
    
    long long int i,j, numGlobalVertices, numGlobalEdges, numParts, node;

    MAP adjlist;
   
    fscanf(fp, "%lld", &numGlobalVertices);
    fscanf(fp, "%lld", &numGlobalEdges);
    fscanf(fp, "%lld", &numParts);


    for(i=0;i<numGlobalVertices;i++){
        long long int j1,j2,n;
        

        fscanf(fp, "%lld%lld%lld", &j1, &j2, &n);

        for (j = 0; j< n; j++) {
            fscanf(fp, "%lld", &node);
            adjlist[i][node] = true;
        }
    }

    fclose(fp);
    
    return adjlist;
}


void compute_cc(MAP adjlist){

    long long int i=0,j,k;
    int num,den;
    unordered_map<long long int, bool>::iterator dest_it;
    double cc,gcc=0;
    vector<long long int> vec;

    for(i=0;i<adjlist.size();i++){

        //cout<<i<<endl;
        for(dest_it=adjlist[i].begin();dest_it!=adjlist[i].end();++dest_it){
        	if(i!=dest_it->first){
        		vec.push_back(dest_it->first);
        	}
            
        }

        num=0;
        den = vec.size();

        for(j=0;j<vec.size();j++){
            for(k=j+1;k<vec.size();k++){
                if(adjlist[vec[j]][vec[k]]){
                    if(i ==0)
                        cout << "DEBUG " << vec[j] <<  "->" << vec[k] << endl; 
                    num++;
                }
            }
        }

        if(den==0 || den==1){
            cc=0;
        }else{
            // cc=2.0*num/( ((double) den) * ((double)(den-1)));
            cc = num;
        }
        
        //gcc+=cc;
        cout<<i<<","<<cc<<endl;
        vec.clear();
        
    }

    //cout<<"Global CC= "<<1.0*gcc/adjlist.size()<<endl;

    return;
}

int main(int argc, char* argv[]){

    char ch;
    const char* infile;
    MAP adjlist;

    while( (ch = getopt( argc, argv, "g:n:")) != -1 )
    {
        if(ch == 'g'){
            infile = optarg;
        }
    }

    if(argc < optind)
    {
        cout<< "./serial_cc -g path-to-graph\n";
        exit(0);
    }

    adjlist = init_graph(infile);

    compute_cc(adjlist);

    return 0;
}
