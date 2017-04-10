#include <stdio.h>
#include <sstream>
#include <fstream>
#include <mpi.h>

typedef struct s_poly {
    double *x;
    int size;
} S_polynomial;

typedef struct s_info {
    int degree;
    double intervalL, intervalR;
    double integration;

} S_info;

int main(int argc, char *argv[]) {

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char buffer[300];
    int position = 0;
    S_polynomial polynomial;
    S_info info;

    //int degree,

    if (rank == 0) {
        std::fstream paramFile;

        /// check arguments
        if (argc != 2) {
            printf("Invalid amount of parameters (%i)\n", argc);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        paramFile.open(argv[1], std::fstream::in);
        if (!paramFile.is_open()) {
            char str[80]{0};
            strcat(str, "params/");
            strcat(str, argv[1]);
            paramFile.open(str, std::fstream::in);
            if (!paramFile.is_open()) {
                printf("Error loading file. Is this a proper filename?\n");
                MPI_Abort(MPI_COMM_WORLD, -2);
            }
        }

        /// get info and set polynomial
        char str[80]{};
        paramFile >> str;
        printf("%s: ", str);
        paramFile >> info.degree;
        printf("%i: ", info.degree);
        paramFile >> str;
        polynomial.size = info.degree + 1;
        printf("\n%s: ", str);
        polynomial.x = (double *) malloc(sizeof(double) * polynomial.size);
        for (int i = info.degree; i >= 0; i--) {
            paramFile >> polynomial.x[i];
            printf("%f ", polynomial.x[i]);
        }
        paramFile >> str;
        printf("\n%s: ", str);
        paramFile >> info.intervalL;
        paramFile >> info.intervalR;
        printf("%f: , %f: \n", info.intervalL, info.intervalR);
        paramFile >> str;
        printf("%s: ", str);
        paramFile >> info.integration;
        printf("%f: \n", info.integration);
        paramFile.close();

        /// pack variables
        MPI_Pack(&polynomial.size, 1, MPI_INT, buffer, 1000, &position, MPI_COMM_WORLD);
        MPI_Pack(polynomial.x, polynomial.size, MPI_DOUBLE, buffer, 1000, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.degree, 1, MPI_INT, buffer, 1000, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.intervalL, 1, MPI_DOUBLE, buffer, 1000, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.intervalR, 1, MPI_DOUBLE, buffer, 1000, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.integration, 1, MPI_DOUBLE, buffer, 1000, &position, MPI_COMM_WORLD);
        MPI_Bcast(buffer, position, MPI_PACKED, 0, MPI_COMM_WORLD);
    }

    MPI_Bcast(buffer, 200, MPI_PACKED, 0, MPI_COMM_WORLD);

    position = 0;
    MPI_Unpack(buffer, 1000, &position, &polynomial.size, 1, MPI_INT, MPI_COMM_WORLD);
        polynomial.x = (double*)malloc(sizeof(double) * polynomial.size);
    MPI_Unpack(buffer, 1000, &position, polynomial.x, polynomial.size, MPI_DOUBLE, MPI_COMM_WORLD);
    MPI_Unpack(buffer, 1000, &position, &info.degree, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(buffer, 1000, &position, &info.intervalL, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    MPI_Unpack(buffer, 1000, &position, &info.intervalR, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    MPI_Unpack(buffer, 1000, &position, &info.integration, 1, MPI_DOUBLE, MPI_COMM_WORLD);





    MPI_Finalize();
    return 0;

}