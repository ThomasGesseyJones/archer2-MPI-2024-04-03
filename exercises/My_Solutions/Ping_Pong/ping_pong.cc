#include "mpi.h"
#include <iostream>

using namespace std;

// Constants
int NUM_TIMES_TO_EXCHANGE = 10000;  // Number of times to communicate back and forth


int main() {
    // Aim is to calculate pi using a parallel algorithm

    // Initialize MPI
    MPI_Init(NULL, NULL);

    // Report in
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    cout << "Hello from process " << rank << " of " << size;

    // Start timing
    MPI_Barrier(MPI_COMM_WORLD); // Line up at the start line
    double tstart = MPI_Wtime(); // Fire the gun and start the clock

    // Finish timing
    MPI_Barrier(MPI_COMM_WORLD); // Wait for everyone to finish
    double tstop = MPI_Wtime(); // Stop the clock
    if (rank == 0) {cout << "Time taken: " << tstop - tstart << " seconds" << endl;}

    MPI_Finalize();
    return 0;
}

