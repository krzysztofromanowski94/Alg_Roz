#include <fstream>
#include <algorithm>
#include <math.h>
#include "mpi.h"
#include <string.h>

int *getVecAmount(int lineAmount, int size){
    int *vecForRank = (int *) calloc(size, sizeof(int));
    int vecLeft = lineAmount; //will use this value to recognise how many vectors are left
    div_t tempDiv; //for counting left vectors
    do {
        tempDiv = div(vecLeft, size); //amount of vectors for each processor
        for (int i = 0; i < size; i++) {
            vecForRank[i] += tempDiv.quot;
        }
        vecLeft = tempDiv.rem; //update left vectors
        if (tempDiv.rem > size)
            vecLeft = tempDiv.rem;
        else {
            int ti = 0; //temporary iterator
            while (vecLeft > 0) {
                vecForRank[(ti++) % size]++;
                vecLeft--; //add 1 to each node until no vectors left
            }
        }
    } while (vecLeft > 0);
    return vecForRank;
}



int main (int argc, char *argv[]) {

	if (argc < 3){
		printf("You know whats wrong...\n");
		return 0;
	}

    MPI_Init(&argc, &argv);
    int rank, size, lineAmount = 0, nodeVecAmount = 0;
    float* elements;
    float average[]= {0,0,0};
    float sum, l;
    double timeTotal, timeReadData, timeProcessData, timeReduceResults;
    int option;// = -1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char filename[30] = "../AR/";
    strcat(filename, argv[1]);
    option = atoi(argv[2]);

    if (option == 0) {

        if (rank == 0) {
            timeReadData = MPI_Wtime(); // start reading time

            std::ifstream myFile;
            myFile.open(filename);

            lineAmount = std::count(std::istreambuf_iterator<char>(myFile),
                                    std::istreambuf_iterator<char>(), '\n');
            printf("Amount of vectors: %i\n", lineAmount);


            elements = (float *) malloc(sizeof(float) * lineAmount * 3);
            int allElemIter = 0;

            myFile.seekg(0, myFile.beg);
            while (!myFile.eof()) {
                char line[256], curVal[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}; //current value
                myFile.getline(line, 256);
                int c = 0; //helps set current char to curVal
                int vi = 0; //temporary vector counter. It resets itself every line
                for (int i = 0; line[i] != 0; i++) { //go through the line while any chars left
                    if (line[i] != 0 && line[i] != 32) { //if there are required characters
                        curVal[c++] = line[i];
                        if (line[i + 1] == 0 || line[i + 1] == 32) { //if next char in line is a whitespace
                            elements[allElemIter++] = atof(curVal);
                            c = 0; //reset current char iterator
                            for (int j = 0; j < 8; j++) //reset current value to be clean for next reading
                                curVal[j] = '\0';
                        }
                    }
                }
            }
            myFile.close();

            int *vecForRank = (int *) calloc(size, sizeof(int));
            int vecLeft = lineAmount; //will use this value to recognise how many vectors are left
            div_t tempDiv; //for counting left vectors
            do {
                tempDiv = div(vecLeft, size); //amount of vectors for each processor
                for (int i = 0; i < size; i++) {
                    vecForRank[i] += tempDiv.quot;
                }
                vecLeft = tempDiv.rem; //update left vectors
                if (tempDiv.rem > size)
                    vecLeft = tempDiv.rem;
                else {
                    int ti = 0; //temporary iterator
                    while (vecLeft > 0) {
                        vecForRank[(ti++) % size]++;
                        vecLeft--; //add 1 to each node until no vectors left
                    }
                }
            } while (vecLeft > 0);

            for (int i = 0; i < size; i++) {
                printf("vectors for rank %i : %i\n", i, vecForRank[i]);
            }

            int stepGate = vecForRank[rank] * 3; //value used for initialising sending start point
            for (int i = 1; i < size; i++) {
                MPI_Send(&(vecForRank[i]), 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(elements + stepGate, vecForRank[i] * 3, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
                stepGate += vecForRank[i] * 3;
            }

            nodeVecAmount = vecForRank[rank];
            free(vecForRank);
            timeReadData = MPI_Wtime() - timeReadData; // end of reading and spreading data
        }

        if (rank != 0) {
            timeReadData = MPI_Wtime();
            MPI_Status status;
            MPI_Recv(&nodeVecAmount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            elements = (float *) malloc(sizeof(float) * nodeVecAmount * 3);
            MPI_Recv(elements, nodeVecAmount * 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
            timeReadData = MPI_Wtime() - timeReadData;
        }
    }
    else {
	timeReadData = MPI_Wtime();
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        int filesize = in.tellg();
        int linesize = 40;
	if (rank == 0){
            printf("filesize: %i ; Amount of vectors: %i\n", filesize, filesize / linesize);
	}
	lineAmount = filesize / linesize;
	in.close();

        int err;
        MPI_File mpiFile;
        err = MPI_File_open( MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &mpiFile );
        if (err) {
            printf("Error opening file %s\n", filename);
            MPI_Abort( MPI_COMM_WORLD, 911 );
        }

        int *vecAmount = getVecAmount(filesize/linesize, size);
        char buf[vecAmount[rank] * linesize];
        for (int i = 0 ; i < vecAmount[rank] * linesize ; i++){
            buf[i] = 0;
        }

        int offset = 0;
        for (int i = 0 ; i < rank ; i++) {
            offset += vecAmount[i] * linesize;
        }
        printf("rank %i offset %i\n", rank, offset);

        err = MPI_File_read_at(mpiFile, offset, buf, vecAmount[rank] * linesize , MPI_CHAR, MPI_STATUS_IGNORE);
        if (err) {
            printf("Error reading file %s\n", filename);
            MPI_Abort( MPI_COMM_WORLD, 911 );
        }

        elements = (float*) malloc (sizeof(float) * vecAmount[rank] * 3);
        printf("elements for rank %i : %i\n", rank, vecAmount[rank] * 3);

        nodeVecAmount = vecAmount[rank];
        int allElemIter = 0;
        char curVal[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}; //current value
        int c = 0; //helps set current char to curVal
        int vi = 0; //temporary vector counter. It resets itself every line
        int i;
        for (i = 0; i < nodeVecAmount * linesize; i++) { //go through the line while any chars left
            if (buf[i] != 10 && buf[i] != 32) { //if there are required characters
                curVal[c++] = buf[i];
                if (buf[i + 1] == 10 || buf[i + 1] == 32 ) { //if next char in line is a whitespace
                    elements[allElemIter++] = atof(curVal);
                    c = 0; //reset current char iterator
                    for (int j = 0; j < 8; j++) //reset current value to be clean for next reading
                        curVal[j] = '\0';
                }
            }
        }
        MPI_File_close(&mpiFile);
	timeReadData = MPI_Wtime() - timeReadData;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// Process data

    timeProcessData = MPI_Wtime();
    l = 0;
    sum = 0;
    for (int i = 0 ; i < nodeVecAmount * 3 ; i++){
        sum += pow(elements[i], 2);
        average[i % 3] += elements[i];
        if (i > 0 && i % 3 == 2 || i == nodeVecAmount * 3 - 1){
            l += sqrt(sum);
            sum = 0;
        }
    }
    timeProcessData = MPI_Wtime() - timeProcessData;

    free(elements);

    timeReduceResults = MPI_Wtime();
    float worldL = 0; // overall length from all nodes
    float worldAverage[] = {0,0,0}; // overall x,y,z average
    MPI_Reduce(&l, &worldL, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    for (int i = 0 ; i < 3 ; i++){
        MPI_Reduce(&(average[i]) ,&(worldAverage[i]), 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    for (int i = 0 ; i < 3 ; i++){
        worldAverage[i] /= lineAmount;
    }
    if (rank == 0 ){
        worldL = worldL / lineAmount;
    }

    timeReduceResults = MPI_Wtime() - timeReduceResults;

    if (rank == 0 ){
        for (int i = 0 ; i < 3 ; i++){
            printf("worldAverage[%i]: %f \n", i, worldAverage[i]);
        }
        printf("worldL: %f\n", worldL);
    }


    double *readData, *processData, *reduceResults;
    if (rank == 0 ){
        readData = (double*)malloc(sizeof(double) * size);
        processData = (double*)malloc(sizeof(double) * size);
        reduceResults = (double*)malloc(sizeof(double) * size);
    }

    MPI_Gather(&timeReadData, 1, MPI_DOUBLE, readData, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&timeProcessData, 1, MPI_DOUBLE, processData, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&timeReduceResults, 1, MPI_DOUBLE, reduceResults, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);


    if (rank == 0){
        for (int i = 0 ; i < size ; i ++){
            printf("from rank: %i, read: %f, process: %f, reduce: %f\n", i, readData[i], processData[i], reduceResults[i]);
        }

        std::ofstream myFile;
        myFile.open("timeResults.txt");
        double totalReadData = 0, totalProcessData = 0, totalReduceResults = 0;

        for (int i = 0 ; i < size ; i++) {
            myFile << "timings (proc " << i << "):\n";
            myFile << "readData:\t" << readData[i] << "\n";
            myFile << "processData:\t" << processData[i] << "\n";
            myFile << "reduceResults:\t" << reduceResults[i] << "\n";
            myFile << "total:\t" <<  readData[i] + processData[i] + reduceResults[i] << "\n\n";

            totalReadData += readData[i];
            totalProcessData += processData[i];
            totalReduceResults += reduceResults[i];
        }

        myFile << "total timings:\n";
        myFile << "readData:\t" << totalReadData << "\n";
        myFile << "processData:\t" << totalProcessData << "\n";
        myFile << "reduceResults:\t" << totalReduceResults << "\n";
        myFile << "total:\t" << totalReadData + totalProcessData + totalReduceResults << "\n";

        myFile.close();
    }


    if (rank == 0) {
        free(readData);
        free(processData);
        free(reduceResults);
    }
    MPI_Finalize();
    return 0;

}
