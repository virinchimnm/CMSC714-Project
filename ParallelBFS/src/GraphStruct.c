#include "GraphStruct.h"

void graphInit(GraphStruct *graph, int num_vtxs, int num_nbors, int num_parts){
	graph->numParts = num_parts;
	graph->numVertices = num_vtxs;
	graph->numNbors = num_nbors;

	if(num_vtxs > 0){
		graph->active = (int *) calloc(sizeof(int), num_vtxs);
		graph->vertexGIDs = (int *) malloc(sizeof(int) * num_vtxs);
		graph->vertexSize = (int *) malloc(sizeof(int) * (num_vtxs));
		graph->partVector = (int *) malloc(sizeof(int) * (num_vtxs));
		graph->vertexWgts = (float *) malloc(sizeof(float) * (num_vtxs));
		graph->nborIndex = (int *) calloc(sizeof(int), (num_vtxs + 1));

		graph->nborGIDs = (int *) malloc(sizeof(int) * num_nbors);
		graph->nborProcs = (int *) malloc(sizeof(int) * num_nbors);
		graph->edgeWgts = (float *) malloc(sizeof(float) * num_nbors);

		graph->initialPartNo2CoreID = (int *) calloc(sizeof(int), graph->numParts);
	}
}

void graphDeinit(GraphStruct * graph) {
    if(graph->numVertices > 0){
    	free(graph->active);
    	free(graph->vertexGIDs);
    	free(graph->vertexSize);
    	free(graph->partVector);
    	free(graph->vertexWgts);
    	free(graph->nborIndex);

    	free(graph->nborGIDs);
    	free(graph->nborProcs);
    	free(graph->edgeWgts);

        free(graph->initialPartNo2CoreID);
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

		printf("%d %d %.1f %d %d ", graph.vertexGIDs[i], graph.partVector[i], graph.vertexWgts[i], graph.vertexSize[i], num_nbors);
		for(j=graph.nborIndex[i]; j<graph.nborIndex[i + 1]; j++){
			printf("%d %.1f ", graph.nborGIDs[j], graph.edgeWgts[j]);
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
	// getNextLine(gdata, line, bufsize);
	// sscanf(line, "%d", &numGlobalEdges);
	// printf("numGlobalEdges = %d\n", numGlobalEdges);

	/* Get the number of partitions  */
	fscanf(fp, "%d", &numParts);
	// getNextLine(gdata, line, bufsize);
	// sscanf(line, "%d", &numParts);
	// printf("numParts = %d\n", numParts);

	/* Allocate arrays to read in entire graph */
	graphInit(graph, numGlobalVertices, numGlobalEdges << 1, numParts);

	// char * token;
	//TODO partition placement???
//	//read current partition mapping to physical cores in pairs of <partNO, coreID>
// 	for (i = 0; i < numParts; i++) {
// 		getNextLine(gdata, line, bufsize);
// //		token = strtok(line, " ");
// //		part = atoi(token);
// //		token = strtok(NULL, " ");
// //		memcpy(graph->partPlacement + part * 5, token, 5);
// 	}
//	graph->partPlacement[5 * numParts] = '\0';

	for (i = 0; i < numGlobalVertices; i++) {
		// getNextLine(gdata, line, bufsize);
		// printf("Line %s", line);

		fscanf(fp, "%d%d%d", &graph->vertexGIDs[i], &graph->partVector[i], &nnbors);
		// sscanf(line, "%d %d %d", &graph->vertexGIDs[i], &graph->partVector[i], &temp);
		// printf("GID %d Part %d #Neighbours %d\n", graph->vertexGIDs[i], graph->partVector[i], temp);
		
		// TODO: Remove these
		graph->vertexWgts[i] = 1.0;
		graph->vertexSize[i] = 1;
	
		
		graph->nborIndex[i + 1] = graph->nborIndex[i] + nnbors;
		for (j = graph->nborIndex[i]; j<graph->nborIndex[i + 1]; j++) {
			// sscanf(line, "%d", &graph->nborGIDs[j]);
			fscanf(fp, "%d", &graph->nborGIDs[j]);
			// printf("%d ", graph->nborGIDs[j]);
			graph->edgeWgts[j] = 1.0;
		}
	}
	return 0;
}
