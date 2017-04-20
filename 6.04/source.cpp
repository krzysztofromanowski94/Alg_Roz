#include <stdio.h>
#include <sstream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <mpi.h>

typedef struct s_poly {
    double *a;
    int size;
} S_polynomial;

typedef struct s_info {
    int degree;
    double intervalL, intervalR;
    int integration;

} S_info;

double evaluate(S_polynomial polynomial,double x){
    double ret = 0;
    for (int i = 0 ; i < polynomial.size ; i++){
        ret += pow(x, i) * polynomial.a[i];
    }
    return ret;
}

double evaluateAnalytic(S_polynomial polynomial,double l, double u){
    double retL = 0;
    double retB = 0;
    for (int i = 0 ; i < polynomial.size ; i++){
        retL += (pow(l, i+1) * polynomial.a[i]) / (i + 1);
        retB += (pow(u, i+1) * polynomial.a[i]) / (i + 1);
    }
    return (retB - retL);
}

int main(int argc, char *argv[]) {

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	/// use MPI_Pack_size
    char *buffer = NULL;
    int position = 0;
    int pack_size = 0;
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
        polynomial.a = (double *) malloc(sizeof(double) * polynomial.size);
        for (int i = 0; i < polynomial.size; i++) {
            paramFile >> polynomial.a[i];
            printf("%f ", polynomial.a[i]);
        }
        paramFile >> str;
        printf("\n%s: ", str);
        paramFile >> info.intervalL;
        paramFile >> info.intervalR;
        printf("%f: , %f: \n", info.intervalL, info.intervalR);
        paramFile >> str;
        printf("%s: ", str);
        paramFile >> info.integration;
        printf("%i: \n", info.integration);
        paramFile.close();


	int temp_pack_size = 0;
	MPI_Pack_size(3, MPI_INT, MPI_COMM_WORLD, &temp_pack_size);
	pack_size += temp_pack_size;
	MPI_Pack_size(2, MPI_DOUBLE, MPI_COMM_WORLD, &temp_pack_size);
	pack_size += temp_pack_size;
	MPI_Pack_size(polynomial.size, MPI_DOUBLE, MPI_COMM_WORLD, &temp_pack_size);
	pack_size += temp_pack_size;
	
        /// pack variables
	buffer = (char*) malloc (sizeof(char) * pack_size);
        MPI_Pack(&polynomial.size, 1, MPI_INT, buffer, pack_size, &position, MPI_COMM_WORLD);
        MPI_Pack(polynomial.a, polynomial.size, MPI_DOUBLE, buffer, pack_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.degree, 1, MPI_INT, buffer, pack_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.intervalL, 1, MPI_DOUBLE, buffer, pack_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.intervalR, 1, MPI_DOUBLE, buffer, pack_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&info.integration, 1, MPI_INT, buffer, pack_size, &position, MPI_COMM_WORLD);
    }


    MPI_Bcast(&pack_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
	buffer = (char*) malloc (sizeof(char) * pack_size);
    MPI_Bcast(buffer, pack_size, MPI_PACKED, 0, MPI_COMM_WORLD);

    /// unpack variables
    if (rank != 0){
	position = 0;
	MPI_Unpack(buffer, pack_size, &position, &polynomial.size, 1, MPI_INT, MPI_COMM_WORLD);
            polynomial.a = (double*)malloc(sizeof(double) * polynomial.size);
	MPI_Unpack(buffer, pack_size, &position, polynomial.a, polynomial.size, MPI_DOUBLE, MPI_COMM_WORLD);
	MPI_Unpack(buffer, pack_size, &position, &info.degree, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(buffer, pack_size, &position, &info.intervalL, 1, MPI_DOUBLE, MPI_COMM_WORLD);
	MPI_Unpack(buffer, pack_size, &position, &info.intervalR, 1, MPI_DOUBLE, MPI_COMM_WORLD);
	MPI_Unpack(buffer, pack_size, &position, &info.integration, 1, MPI_INT, MPI_COMM_WORLD);
    }

    if (rank == 0){
        //printf("%f: \n", evaluate(polynomial, 12));
        //printf("Eval analitic: %f \n", evaluateAnalytic(polynomial, info.intervalL, info.intervalR));
	double length =  info.intervalR - info.intervalL;
	printf("length: %f intervals: %i interval length: %f\n", length, info.integration, length / info.integration);
	int *intervalForRank;
	
    }

    int *intervalForRank = (int*)malloc(sizeof(int) * size);
    int quot = info.integration / size;
    int rem = info.integration % size;
    for (int i = 0 ; i < size ; i++)
	intervalForRank[i] = quot;
    for (int i = 0 ; rem > 0 ; i++){
	intervalForRank[i]++;
	rem--;
    }

    int l = 0;
    int u = 0;

    for (int i = 0 ; i < rank; i++){
	l += intervalForRank[i];
	//printf("rank %i\n", rank);
    }

    //printf("rank %i l: %i\n",rank,  l);
    int lowRank = l, uppRank = 0;
    for (int i = 0 ; i < rank + 1; i++){
	uppRank += intervalForRank[i];
    }
    uppRank--;
    printf("rank %i l: %i ; u: %i\n", rank, lowRank, uppRank);
    
    



    free (polynomial.a);
    free (intervalForRank);
    MPI_Finalize();
    return 0;

}
