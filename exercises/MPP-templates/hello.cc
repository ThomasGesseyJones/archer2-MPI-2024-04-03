#include "mpi.h"
#include <iostream>

using namespace std;

int hello_world() {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // note pass variable to give output to as reference
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // as outputs used for error code
    printf("Hello world from process %d of %d\n", rank, size);
    MPI_Finalize();
    return 0;
}


int main() {
    hello_world();
    return 0;
}

