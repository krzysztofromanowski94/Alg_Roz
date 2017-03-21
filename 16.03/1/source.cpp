#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include "mpi.h"

int main (int argc, char *argv[]){
	
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0){
	std::ifstream myFile;
	myFile.open("../AR/v01.dat");

	int linesAmount = std::count(std::istreambuf_iterator<char>(myFile),
		std::istreambuf_iterator<char>(), '\n');

	std::cout << linesAmount << "\n";


	char line[256], curVal[8];
	myFile.seekg(0, myFile.beg);
	myFile.getline(line, 256);

	int c = 0;
	for (int i = 0 ; line[i] != 0 ; i++){
		if (line[i] != 0 && line[i] != 32){
			curVal[c++] = line[i];
			if (line[i+1] == 0 || line[i+1] == 32){
				float f = atof(curVal);
				std::cout << f << "\n";
				c = 0;
			}
		}
	}

	}
	
	
	myFile.close();
	MPI_Finalize();

}
