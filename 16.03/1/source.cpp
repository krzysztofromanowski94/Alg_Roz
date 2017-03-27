#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <cstdlib>
#include "mpi.h"

typedef struct s_v3 {
    float x[3];
} Vec3;

int main (int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    int rank, size, lineAmount, nodeVecAmount;
    float* nodeElem;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	if (rank == 0){
		std::ifstream myFile;
        myFile.open("../AR/v05.dat");
        //myFile.open("../AR/small_test.dat");
		lineAmount = std::count(std::istreambuf_iterator<char>(myFile),
			std::istreambuf_iterator<char>(), '\n');
		std::cout << lineAmount << "\n";


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


        int nodeVecAmount = vecForRank[rank];
        float l = 0; //average length of vector
        float sum = 0; // square sum of single vector elements
        float average[] = {0,0,0};

        for (int i = 0 ; i < nodeVecAmount * 3 ; i++){
            sum += pow(allVec[i], 2);
            average[i % 3] += allVec[i];
            if (i > 0 && i % 3 == 0){
                l += sqrt(sum);
                sum = 0;
            }
        }

        float N = nodeVecAmount;
        l /= N;
        average[0] /= N;
        average[1] /= N;
        average[2] /= N;


        // get result from nodes


		free(allVec);
        free(vecForRank);
	}

    if (rank != 0){
        MPI_Status status;
        int nodeVecAmount;
        MPI_Recv(&nodeVecAmount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        nodeElem = (float*)malloc(sizeof(float) * nodeVecAmount*3);
        MPI_Recv(nodeElem, nodeVecAmount*3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);

        float l = 0; //average length of vector
        float sum = 0; // square sum of single vector elements
        float average[] = {0,0,0};

        for (int i = 0 ; i < nodeVecAmount * 3 ; i++){
            sum += pow(nodeElem[i], 2);
            average[i % 3] += nodeElem[i];
            if (i > 0 && i % 3 == 0){
                l += sqrt(sum);
                sum = 0;
            }
        }

        float N = nodeVecAmount;
        l /= N;
        average[0] /= N;
        average[1] /= N;
        average[2] /= N;

        //send result to node


        free(nodeElem);
    }

    MPI_Finalize();
	return 0;

}
