#include <time.h>
#include <limits.h>
#include <mpi.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <GraphDist.h>

typedef unsigned int uint;

static int myRank;

using namespace std;

void parseCommandLineArguments(int argc,char *argv[], int &root, std::string &ip, std::string &op)
{
	root = 0;
	ip = "input/sample_input.txt";
	op = "";
	for(int i = 1; i < argc; i++)
	{
		//cout << (string(argv[i]) == "-o") << endl;
		if(std::string(argv[i]) == "-r")
		{
			root = std::stoi(argv[i+1]);
			std::cout << "root set to " << root << std::endl;
		}
		else if(std::string(argv[i]) == "-i")
		{
			ip = std::string(argv[i+1]);
			std::cout << "input file path " << ip << std::endl;
		}
		else if(std::string(argv[i]) == "-o")
		{
			op = std::string(argv[i+1]);
			std::cout << "output file path " << op << std::endl;
		}
	}
}

int main(int argc, char *argv[]) {

	int numParts;
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	MPI_Comm_size(comm, &numParts);

	cout << "Number of MPI processes " << numParts << endl;
	
	int srcRank = 0;
	int srcLid = 0;
	std::string fname, ofname;

	if(myRank == srcRank)
	{
		parseCommandLineArguments(argc,argv,srcLid,fname,ofname);
	}

	GraphStruct localGraph;

	// TODO: Maybe we should be able to replace this by graphLoad(&localGraph, subGraphFile)
	getSubGraph(comm, &localGraph, fname, numParts);
	srand(time(NULL));

	//Have to call the function to count the triangles
	//Print the output to a output file.

	//globalDist.clear();
	graphDeinit(&localGraph);
	MPI_Finalize();
	return 0;

}