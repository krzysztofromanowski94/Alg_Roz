#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include "mpi.h"

typedef struct s_v3 {
    float x[3];
} Vec3;

int main (int argc, char *argv[]) {

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        std::vector<Vec3> vectorList;
        std::ifstream myFile;
        myFile.open("../AR/v01.dat");
        int linesAmount = std::count(std::istreambuf_iterator<char>(myFile),
                                     std::istreambuf_iterator<char>(), '\n');

        std::cout << linesAmount << "\n";

        myFile.seekg(0, myFile.beg);
        while(!myFile.eof()) {
            char line[256], curVal[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}; //current value
            myFile.getline(line, 256);
            int c = 0; //helps set current char to curVal
            int vi = 0; //temporary vector counter. It resets itself every line
            Vec3 tempV; //temporary vector
            for (int i = 0 ; line[i] != 0 ; i++) { //go through the line while any chars left
                if (line[i] != 0 && line[i] != 32) { //if there are required characters
                    curVal[c++] = line[i];
                    if (line[i+1] == 0 || line[i+1] == 32) { //if next char in line is a whitespace
                        tempV.x[vi++] = atof(curVal); //save currently readed value to the temporary vector
                        if (vi == 3)
                            vectorList.push_back(tempV);
                        c = 0; //reset current char iterator
                        for (int j = 0 ; j < 8 ; j++) //reset current value to be clean for next reading
                            curVal[j] = '\0';
                    }
                }
            }
        }
        myFile.close();

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


    }


    MPI_Finalize();

}
