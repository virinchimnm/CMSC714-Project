#include <string>
#include <sstream>
#include <iostream>

#include "GraphDist.h"

void hashingPartitioning(GraphStruct *globalGraph, int numParts){
	int i;

	for(i=0; i<globalGraph->numVertices; i++){
		int part = globalGraph->vertexGIDs[i] % numParts;
		globalGraph->partVector[i] = part;
	}
	globalGraph->numParts = numParts;
}


// This functions sends a sub graph to the destination process
// TODO: Remove this function dependency with parallel read of subgraphs
void graphSend(MPI_Comm comm, GraphStruct sub_graph, int dest_proc) {
	int num_procs;
	MPI_Comm_size(comm, &num_procs);

	int send_count[4];
	int ack = 0, ack_tag = 5, count_tag = 10, id_tag = 15;

	send_count[0] = sub_graph.numVertices;
	send_count[1] = sub_graph.numNbors;
	send_count[2] = sub_graph.numParts;

	MPI_Send(send_count, 4, MPI_INT, dest_proc, count_tag, comm);
	MPI_Recv(&ack, 1, MPI_INT, dest_proc, ack_tag, comm, MPI_STATUS_IGNORE);

	if (send_count[0] > 0) {
		MPI_Send(sub_graph.vertexGIDs, send_count[0], MPI_INT,
				dest_proc, id_tag, comm);
		MPI_Send(sub_graph.nborIndex, send_count[0] + 1, MPI_INT, dest_proc,
				id_tag + 1, comm);
		MPI_Send(sub_graph.partVector, send_count[0], MPI_INT, dest_proc,
				id_tag + 2, comm);

		if (send_count[1] > 0) {
			MPI_Send(sub_graph.nborGIDs, send_count[1], MPI_INT,
					dest_proc, id_tag + 5, comm);
			MPI_Send(sub_graph.nborProcs, send_count[1], MPI_INT, dest_proc,
					id_tag + 6, comm);
		}
	}
}

// This functions received a sub graph from process 0
// TODO: Remove this function dependency with parallel read of subgraphs
void graphRecv(MPI_Comm comm, GraphStruct * local_graph){
	int num_procs;
	MPI_Comm_size(comm, &num_procs);
	int send_count[4];
	int ack = 0, ack_tag = 5, count_tag = 10, id_tag = 15;

	MPI_Recv(send_count, 4, MPI_INT, 0, count_tag, comm, MPI_STATUS_IGNORE);
	MPI_Send(&ack, 1, MPI_INT, 0, ack_tag, comm);

	if (send_count[0] > 0) {
		graphInit(local_graph, send_count[0], send_count[1], send_count[2]);

		MPI_Recv(local_graph->vertexGIDs, send_count[0], MPI_INT, 0,
				id_tag, comm, MPI_STATUS_IGNORE);
		MPI_Recv(local_graph->nborIndex, send_count[0] + 1, MPI_INT, 0,
				id_tag + 1, comm, MPI_STATUS_IGNORE);
		MPI_Recv(local_graph->partVector, send_count[0], MPI_INT, 0, id_tag + 2,
				comm, MPI_STATUS_IGNORE);
		if (send_count[1] > 0) {
			MPI_Recv(local_graph->nborGIDs, send_count[1], MPI_INT, 0,
					id_tag + 5, comm, MPI_STATUS_IGNORE);
			MPI_Recv(local_graph->nborProcs, send_count[1], MPI_INT, 0,
					id_tag + 6, comm, MPI_STATUS_IGNORE);
		}
	}

	MPI_Recv(&ack, 1, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
	if (ack < 0) {
		int my_rank;
		MPI_Comm_rank(comm, &my_rank);
		printf("oops! Rank %d receive subgraph from rank 0 failed!\n", my_rank);
	}
}

// Maybe we should be able to remove this function altogether as long as
// each process reads it's own graph.
//distribute the graph according to the partition number (rank-i responsible for partition i);
void graphDistribute(MPI_Comm comm, GraphStruct global_graph, GraphStruct * local_graph)
{
	int num_procs;
	MPI_Comm_size(comm, &num_procs);

	int i, j, proc_id, vtx_lid, nnbors, nbor_index, nbor_proc;
	
	//GraphStruct *sub_graphs = (GraphStruct *) malloc(sizeof(GraphStruct) * num_procs);
	std::vector<GraphStruct> sub_graphs(num_procs);
	
	for (i = 0; i < num_procs; i++) 
	{
		sub_graphs[i].numVertices = 0;
		sub_graphs[i].numNbors = 0;
	}

	int num_parts_per_proc;
	if(global_graph.numParts > num_procs)
	{
		num_parts_per_proc = global_graph.numParts / num_procs + global_graph.numParts % num_procs; //TODO
	}else{
		num_parts_per_proc =  1;
	}

	//int *global_part_vector = malloc(sizeof(int) * global_graph.numVertices);
	std::vector<int> global_part_vector(global_graph.numVertices);
	
	for (i = 0; i < global_graph.numVertices; i++) 
	{
		proc_id = global_graph.partVector[i] / num_parts_per_proc;
		sub_graphs[proc_id].numVertices++;
		sub_graphs[proc_id].numNbors += (global_graph.nborIndex[i + 1] - global_graph.nborIndex[i]);
		global_part_vector[global_graph.vertexGIDs[i]] = global_graph.partVector[i];
	}

	for (i = 0; i < num_procs; i++)
	{
		graphInit(&sub_graphs[i], sub_graphs[i].numVertices,sub_graphs[i].numNbors, global_graph.numParts);
		sub_graphs[i].numVertices = 0;
		sub_graphs[i].numNbors = 0;
	}

	for (i = 0; i < global_graph.numVertices; i++) 
	{
		proc_id = global_graph.partVector[i] / num_parts_per_proc;
		vtx_lid = sub_graphs[proc_id].numVertices ++;
		sub_graphs[proc_id].vertexGIDs[vtx_lid] = global_graph.vertexGIDs[i];
		sub_graphs[proc_id].partVector[vtx_lid] = global_graph.partVector[i];

		nnbors = global_graph.nborIndex[i + 1] - global_graph.nborIndex[i];
		sub_graphs[proc_id].nborIndex[vtx_lid + 1] = sub_graphs[proc_id].nborIndex[vtx_lid] + nnbors;
		for (j = global_graph.nborIndex[i]; j < global_graph.nborIndex[i + 1]; j++)
		{
			nbor_index = sub_graphs[proc_id].numNbors ++;
			nbor_proc = global_part_vector[global_graph.nborGIDs[j]] / num_parts_per_proc;

			sub_graphs[proc_id].nborProcs[nbor_index] = nbor_proc;
			sub_graphs[proc_id].nborGIDs[nbor_index] = global_graph.nborGIDs[j];
		}
	}
	
	//free(global_part_vector);
	*local_graph = sub_graphs[0];

	for (i = 1; i < num_procs; i++)
	{
		graphSend(comm, sub_graphs[i], i);
		graphDeinit(&sub_graphs[i]);
	}
	
	//free(sub_graphs);

	int ack = 0;
	for (i = 1; i < num_procs; i++)
	{
		MPI_Send(&ack, 1, MPI_INT, i, 0, comm);
	}
}

//The graph is initial partition by hashing vertex gid
void getSubGraph(MPI_Comm comm, GraphStruct *sub_graph, std::string fname, int numParts){
	int my_rank;
	MPI_Comm_rank(comm, &my_rank);

	if (my_rank == 0) {
		FILE *fp = fopen(fname.c_str(), "r");
		if (fp == NULL) {
			printf("File %s not found\n", fname.c_str());
		}
		GraphStruct globalGraph;
		graphLoad(&globalGraph, fp);

		// This functions creates a partition by assigning n/p vertices to each processors
		// hashingPartitioning(&globalGraph, numParts);	//hashing partitioning

		graphDistribute(comm, globalGraph, sub_graph);
		graphDeinit(&globalGraph);
		fclose(fp);
	} else {
		graphRecv(comm, sub_graph);
	}
}


