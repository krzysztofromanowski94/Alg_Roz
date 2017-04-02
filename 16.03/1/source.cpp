#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <cstdlib>
#include "mpi.h"



int main (int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    int rank, size, lineAmount, nodeVecAmount;
    float* nodeElem;
    float average[]= {0,0,0};
    float sum, l;
    double timeTotal, timeReadData, timeProcessData, timeReduceResults;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0){
        timeReadData = MPI_Wtime(); // start reading time

        std::ifstream myFile;
        myFile.open("../AR/v01.dat");

        lineAmount = std::count(std::istreambuf_iterator<char>(myFile),
                                std::istreambuf_iterator<char>(), '\n');
        printf("Amount of vectors: %i\n", lineAmount);


        float* allVec = (float*)malloc(sizeof(float) * lineAmount * 3);
        int allElemIter = 0;

        myFile.seekg(0, myFile.beg);
        while(!myFile.eof()) {
            char line[256], curVal[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}; //current value
            myFile.getline(line, 256);
            int c = 0; //helps set current char to curVal
            int vi = 0; //temporary vector counter. It resets itself every line
            for (int i = 0 ; line[i] != 0 ; i++) { //go through the line while any chars left
                if (line[i] != 0 && line[i] != 32) { //if there are required characters
                    curVal[c++] = line[i];
                    if (line[i+1] == 0 || line[i+1] == 32) { //if next char in line is a whitespace
                        allVec[allElemIter++] = atof(curVal);
                        c = 0; //reset current char iterator
                        for (int j = 0 ; j < 8 ; j++) //reset current value to be clean for next reading
                            curVal[j] = '\0';
                    }
                }
            }
        }
        myFile.close();

        int* vecForRank = (int*)calloc(size, sizeof(int));
        int vecLeft = lineAmount; //will use this value to recognise how many vectors are left
        div_t tempDiv; //for counting left vectors
        do{
            tempDiv = div(vecLeft, size); //amount of vectors for each processor
            for (int i = 0 ; i < size ; i++){
                vecForRank[i] += tempDiv.quot;
            }
            vecLeft = tempDiv.rem; //update left vectors
            if (tempDiv.rem > size)
                vecLeft = tempDiv.rem;
            else {
                int ti = 0; //temporary iterator
                while (vecLeft > 0){
                    vecForRank[(ti++) % size]++;
                    vecLeft--; //add 1 to each node until no vectors left
                }
            }
        }while (vecLeft > 0);

        int stepGate = vecForRank[rank] * 3; //value used for initialising sending start point
        for (int i = 1 ; i < size ; i++){
            MPI_Send(&(vecForRank[i]), 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(allVec+stepGate, vecForRank[i]*3, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            stepGate += vecForRank[i] * 3;
        }

        timeReadData = MPI_Wtime() - timeReadData; // end of reading and spreading data
        timeProcessData = MPI_Wtime();

        sum = 0;
        l = 0;
        int nodeVecAmount = vecForRank[rank];
        for (int i = 0 ; i < nodeVecAmount * 3 ; i++){
            sum += pow(allVec[i], 2);
            average[i % 3] += allVec[i];
            if (i > 0 && i % 3 == 2 || i == nodeVecAmount *3 - 1){
                l += sqrt(sum);
                sum = 0;
            }
        }

        timeProcessData = MPI_Wtime() - timeProcessData;

        free(allVec);
        free(vecForRank);
    }

    if (rank != 0){
        timeReadData = MPI_Wtime();
        MPI_Status status;
        int nodeVecAmount;
        MPI_Recv(&nodeVecAmount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        nodeElem = (float*)malloc(sizeof(float) * nodeVecAmount*3);
        MPI_Recv(nodeElem, nodeVecAmount*3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        timeReadData = MPI_Wtime() - timeReadData;


        timeProcessData = MPI_Wtime();
        l = 0;
        sum = 0;
        for (int i = 0 ; i < nodeVecAmount * 3 ; i++){
            sum += pow(nodeElem[i], 2);
            average[i % 3] += nodeElem[i];
            if (i > 0 && i % 3 == 2 || i == nodeVecAmount * 3 - 1){
                l += sqrt(sum);
                sum = 0;
            }
        }
        timeProcessData = MPI_Wtime() - timeProcessData;
        free(nodeElem);
    }

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
        worldL /= lineAmount;
    }

    timeReduceResults = MPI_Wtime() - timeReduceResults;

    if (rank == 0 ){
        for (int i = 0 ; i < 3 ; i++){
            printf("worldAverage[%i]: %f \n", i, worldAverage[i]);
        }
        printf("worldL: %f\n", worldL);
        //printf("Overall time: %f\n", MPI_Wtime() - myTime);
    }

    printf("rank: %i, readData: %f, processData: %f, reduceResults: %f\n", rank, timeReadData, timeProcessData, timeReduceResults);


    MPI_Finalize();
    return 0;

}
