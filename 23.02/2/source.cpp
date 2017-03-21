#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

  int rank, worldSize, rankRand;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

  if(rank != 0){
    srand(time(NULL) / rank);
    rankRand = rand() % worldSize + 1;
    MPI_Send(&rankRand, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    int sum;
    MPI_Recv(&sum, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::cout << "Node " << rank << " received sum: " << sum << std::endl;
  }else{
    int sum = 0, buf;
    MPI_Status status;
    for (int i = 0 ; i < worldSize - 1; i++){
      MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      std::cout << "Recived random number: " << buf << " from " << status.MPI_SOURCE << std::endl;
      sum += buf;
    }
    for (int i = 1 ; i < worldSize ; i++){
      MPI_Send(&sum, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

  }


  MPI::Finalize();
  return 0;
}
