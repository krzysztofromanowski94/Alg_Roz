#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

  int rank, worldSize;
  double **matrix = NULL, **receivedMatrix = NULL;
  double *data = NULL, *receivedData = NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

  if (worldSize % 2 != 0){
    if (rank == 0)
      std::cout << "Amount of processors not even. Abort...\n";
    MPI_Finalize();
    return 0;
  }

  srand(time(NULL) / (rank + 1));

  data = (double*)malloc(worldSize*worldSize*sizeof(double));
  matrix = (double**)malloc(worldSize * sizeof(double*));
  receivedData = (double*)malloc(worldSize*worldSize*sizeof(double));
  receivedMatrix = (double**)malloc(worldSize * sizeof(double*));
  for (int i = 0 ; i < worldSize ; i++){
    matrix[i] = &(data[i * worldSize]);
    receivedMatrix[i] = &(receivedData[i * worldSize]);
  }

  for (size_t i = 0; i < worldSize; i++) {
    for (size_t j = 0; j < worldSize; j++) {
      matrix[i][j] = (double)rand() / (double)RAND_MAX;
    }
  }

  if (rank % 2 == 0){
//
//    for (size_t i = 0; i < worldSize; i++) {
//      for (size_t j = 0; j < worldSize; j++) {
//        std::cout << matrix[i][j] << " ";
//      }
//      std::cout << std::endl;
//    }

    MPI_Status status;
    MPI_Send(*matrix, worldSize*worldSize, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
    MPI_Recv(*receivedMatrix, worldSize*worldSize, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &status);
  }else{
    MPI_Status status;
    MPI_Recv(*receivedMatrix, worldSize*worldSize, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);
    MPI_Send(*matrix, worldSize*worldSize, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
  }

  std::stringstream mStr;
    mStr << "Rank " << rank << " generated matrix:\n";
    for (int i = 0 ; i < worldSize ; i++){
      for (int j = 0 ; j < worldSize ; j++){
      mStr << matrix[i][j] << " ";
      }
      mStr << "\n";
    }

    mStr << "\nAnd received matrix from " << (rank % 2 == 0 ? rank + 1 : rank -1) << "\n";
    for (int i = 0 ; i < worldSize ; i++){
      for (int j = 0 ; j < worldSize ; j++){
      mStr << receivedMatrix[i][j] << " ";
      }
      mStr << "\n";
    }


    std::ofstream myFile;
    std::stringstream filename;
    filename << "log.proc_" << rank;
    myFile.open (filename.str().c_str());
    myFile << mStr.str();
    myFile.close();


  if (data != NULL){
    free(data);
  }
  if (receivedMatrix[0] != NULL){
    free(receivedMatrix[0]);
    free (receivedMatrix);
  }

  MPI_Finalize();
  return 0;
}
