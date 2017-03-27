#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
//#include <vector>
//#include <math.h>
#include <cstdlib>
#include "mpi.h"

typedef struct s_v3 {
    float x[3];
} Vec3;

int main (int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    int rank, size, lineAmount, nodeVecAmount;
    float* nodeElem;
    //std::vector<Vec3> vectorList;
    Vec3* vectorList;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	if (rank == 0){
		std::ifstream myFile;
        //myFile.open("../AR/v01.dat");
        myFile.open("../AR/small_test.dat");
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
	        getchar();	
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

        int stepGate = vecForRank[rank]; //value used for initialising sending start point
        for (int i = 1 ; i < size ; i++){
            std::cout << "vecForRank[" << i << "]: " << vecForRank[i] << "\n";
            int test = 2;
            MPI_Send(&test, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            //MPI_Send(&(vecForRank[i]), 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            
            //MPI_Send(allVec+stepGate, vecForRank[i]*3, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            //stepGate += vecForRank[i];
        }

        //nodeVecAmount = vecForRank[rank];
        //Vec3 tempVec;
        
		//vectorList = (Vec3*)malloc(sizeof(Vec3) * vecForRank[rank]);
        for (int i = 0 ; i < vecForRank[rank] * 3 ; i++){
            //tempVec.x[i % 3] = allVec[i];
            //vectorList[i/3].x[i % 3] = allVec[i];
            //std::cout << vectorList[i/3].x[i % 3] << "\n";
            //if (i > 0 && ((i % 3) == 0))
            //    vectorList.push_back(tempVec);
        }
		
        myFile.close();
//		free(allVec);
//        free(vecForRank);
	}

    if (rank != 0){ //ToDo
        MPI_Status* status;
        std::cout << "Here's good\n";
        int test2;
        MPI_Recv(&test2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, status);
        //MPI_Recv(&nodeVecAmount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, status);
        //std::cout << "MPI_Recv nodeVecEmount: " << nodeVecAmount << "\n";
        //std::cout << "node vec amount: " << nodeVecAmount << "\n";
        //nodeElem = (float*)malloc(sizeof(float) * nodeVecAmount*3);
        //MPI_Recv(nodeElem, nodeVecAmount*3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, status);

        //for (int i = 0 ; i < nodeVecAmount * 3 ; i++)
        //    std::cout << nodeElem[i] << "\n";



/*        Vec3 tempVec;
        for (int i = 0 ; i < nodeVecAmount*3 ; i++){
            tempVec.x[i % 3] = nodeElem[i];
            std::cout << i%3;
            if (i > 0 && ((i % 3) == 2)) {
                //vectorList.push_back(tempVec); //vector is not thread-safe
                std::cout << "i";
            }
        }*/

        //std::cout << "vectorList.size(): " << vectorList.size() << "\n";
//
//        for (int i = 0 ; i < vectorList.size() ; i++){
//            std::cout << vectorList[i].x[0] << "\n";
//        }


        //free(nodeElem);
    }



 /*   if (rank == 0) {
        std::vector<Vec3> vectorList;



        float l = 0; //average length of vector
        float average[] = {0,0,0};
        for (int i = 0 ; i < vectorList.size() ; i++) {
            int j = 0; //going through curVec
            float sum = 0; //sum of squares
            for (Vec3 curVec = vectorList[i] ; j < 3 ; j++) {
                sum += pow(curVec.x[j], 2);
                average[j] += curVec.x[j];
            }
            l += sqrt(sum);
        }

        float N = vectorList.size();
        l /= N;
        average[0] /= N;
        average[1] /= N;
        average[2] /= N;

        std::cout << l << "\n" << average[0] << "\n" ;


    }*/
    //free(nodeElem);
    MPI_Finalize();
	return 0;

}
