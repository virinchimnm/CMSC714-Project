#include "GraphStruct.h"

void graphInit(GraphStruct *graph, int num_vtxs, int num_nbors, int num_parts){
	graph->numParts = num_parts;
	graph->numVertices = num_vtxs;
	graph->numNbors = num_nbors;

	if(num_vtxs > 0){
		graph->vertexGIDs = (int *) malloc(sizeof(int) * num_vtxs);
		graph->partVector = (int *) malloc(sizeof(int) * (num_vtxs));
		graph->nborIndex = (int *) calloc(sizeof(int), (num_vtxs + 1));

		graph->nborGIDs = (int *) malloc(sizeof(int) * num_nbors);
		graph->nborProcs = (int *) malloc(sizeof(int) * num_nbors);
	}
}

void graphDeinit(GraphStruct * graph) {
    if(graph->numVertices > 0){
    	free(graph->vertexGIDs);
    	free(graph->partVector);
    	free(graph->nborIndex);

    	free(graph->nborGIDs);
    	free(graph->nborProcs);
    }
    graph->numVertices = 0;
    graph->numNbors = 0;
    graph->numParts = 0;
}

void graphPrint(GraphStruct graph){
	printf("num_vtx=%d, num_nbors=%d, num_parts=%d\n", graph.numVertices, graph.numNbors, graph.numParts);

	int i, j, num_nbors;
	for(i=0; i<graph.numVertices; i++){
		num_nbors = graph.nborIndex[i + 1] - graph.nborIndex[i];

		printf("%d %d %d ", graph.vertexGIDs[i], graph.partVector[i], num_nbors);
		for(j=graph.nborIndex[i]; j<graph.nborIndex[i + 1]; j++){
			printf("%d ", graph.nborGIDs[j]);
		}
		printf("\n");
	}
}

int graphLoad(GraphStruct * graph, FILE * fp) {
	int numGlobalVertices, numGlobalEdges, numParts;
	int i, j, nnbors;

	/* Get the number of vertices */
	fscanf(fp, "%d", &numGlobalVertices);
	// printf("numGlobalVertices = %d\n", numGlobalVertices);

	/* Get the number of edges  */
	fscanf(fp, "%d", &numGlobalEdges);
	// printf("numGlobalEdges = %d\n", numGlobalEdges);

	/* Get the number of partitions  */
	fscanf(fp, "%d", &numParts);
	graph->numParts = numParts;
	// printf("numParts = %d\n", numParts);

	/* Allocate arrays to read in entire graph */
	graphInit(graph, numGlobalVertices, numGlobalEdges << 1, numParts);

	for (i = 0; i < numGlobalVertices; i++) {
		fscanf(fp, "%d%d%d", &graph->vertexGIDs[i], &graph->partVector[i], &nnbors);
		// printf("GID %d Part %d #Neighbours %d\n", graph->vertexGIDs[i], graph->partVector[i], nnbors);

		graph->nborIndex[i + 1] = graph->nborIndex[i] + nnbors;
		for (j = graph->nborIndex[i]; j<graph->nborIndex[i + 1]; j++) {
			fscanf(fp, "%d", &graph->nborGIDs[j]);
			// printf("%d ", graph->nborGIDs[j]);
		}
	}
	return 0;
}

//This function is helpful on reading the graph for counting triangles.
//Here the adjacency matrix of the graph is modified in such a way that the 
//for vertices u < v and for an 'uv': Only the adjacency list of u depicts an
//edge between the vertices. This is helpful because this removes the duplicate
//counts of the triangle.
int graphLoadForCountingTriangles(GraphStruct * graph, FILE * fp){

	int numGlobalVertices, numGlobalEdges, numParts;
	int i, j, nnbors;

	/* Get the number of vertices */
	fscanf(fp, "%d", &numGlobalVertices);
	// printf("numGlobalVertices = %d\n", numGlobalVertices);

	/* Get the number of edges  */
	fscanf(fp, "%d", &numGlobalEdges);
	// printf("numGlobalEdges = %d\n", numGlobalEdges);

	/* Get the number of partitions  */
	fscanf(fp, "%d", &numParts);
	graph->numParts = numParts;
	// printf("numParts = %d\n", numParts);

	/* Allocate arrays to read in entire graph */
	graphInit(graph, numGlobalVertices, numGlobalEdges << 1, numParts);

	for (i = 0; i < numGlobalVertices; i++) {
		fscanf(fp, "%d%d%d", &graph->vertexGIDs[i], &graph->partVector[i], &nnbors);
		// printf("GID %d Part %d #Neighbours %d\n", graph->vertexGIDs[i], graph->partVector[i], nnbors);

		j = graph->nborIndex[i];
		int k =0, connectedVertex=0;

		printf("\n%d : ", i);

		for(k=0; k<nnbors; k++){
			fscanf(fp, "%d", &connectedVertex);
			if(i<connectedVertex){
				graph->nborGIDs[j] = connectedVertex;
				j++;
				printf("%d ", connectedVertex);
			}
		}

		graph->nborGIDs[j] = j - graph->nborIndex[i];
		//for (j = graph->nborIndex[i]; j<graph->nborIndex[i + 1]; j++) {
		//	fscanf(fp, "%d", &graph->nborGIDs[j]);
			// printf("%d ", graph->nborGIDs[j]);
		//}
		graph->nborIndex[i + 1] = j;
	}
	return 0;

}
