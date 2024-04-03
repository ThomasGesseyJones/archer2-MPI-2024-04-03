#include "mpi.h"
#include <iostream>

using namespace std;

// Constants
int NUM_TIMES_TO_EXCHANGE = 10000;  // Number of times to communicate back and forth


int main() {
    // Aim is to calculate pi using a parallel algorithm

    // Initialize MPI
    MPI_Init(NULL, NULL);
    MPI_Comm comm;
    comm = MPI_COMM_WORLD;

    // Report in
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    cout << "Hello from process " << rank << " of " << size << endl;

    // Start timing
    MPI_Barrier(comm); // Line up at the start line
    double tstart = MPI_Wtime(); // Fire the gun and start the clock

    // Pass array back and forth
    int array_size = 100;
    int array[array_size];
    for (int idx=0; idx<array_size; idx++) {array[idx] = idx;}

    MPI_Status status;
    for (int exchange_idx=0; exchange_idx<NUM_TIMES_TO_EXCHANGE; exchange_idx++){
        if (rank == 0){
            MPI_Ssend(array, array_size, MPI_INT, 1, 0, comm)
            MPI_Recv(array, array_size, MPI_INT, 1, 0, comm, &status);
        } else if (rank == 1){
            MPI_Recv(array, array_size, MPI_INT, 0, 0, comm, &status);
            MPI_Ssend(array, array_size, MPI_INT, 0, 0, comm);
        }
    }

    // Finish timing
    MPI_Barrier(comm); // Wait for everyone to finish
    double tstop = MPI_Wtime(); // Stop the clock
    if (rank == 0) {cout << "Time taken: " << tstop - tstart << " seconds" << endl;}

    MPI_Finalize();
    return 0;
}

