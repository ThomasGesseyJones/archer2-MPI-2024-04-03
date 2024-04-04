#include "mpi.h"
#include <iostream>

using namespace std;


//Constants
const int NUM_LOOPS = 100000;

int main() {
    // Initialize MPI
    MPI_Init(NULL, NULL);
    MPI_Comm comm;
    comm = MPI_COMM_WORLD;

    // Report in
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    cout << "Hello from process " << rank << " of " << size << endl;

    int total;

    // Start timing
    MPI_Barrier(comm); // Line up at the start line
    double tstart = MPI_Wtime(); // Fire the gun and start the clock

    for (int loop_idx=0; loop_idx<NUM_LOOPS; loop_idx++) {
        // Start with value derived from rank
        int to_pass = (rank + 1) * (rank + 1); // Harder calculation

        // Use reduce to get total on all nodes
        MPI_Allreduce(&to_pass, &total, 1, MPI_INT, MPI_SUM, comm);
    }

    // Print the total
    cout << "Process " << rank << " has total " << total << endl;

    // Finish timing
    MPI_Barrier(comm); // Wait for everyone to finish
    double tstop = MPI_Wtime(); // Stop the clock
    if (rank == 0) { cout << "Time taken: " << tstop - tstart << " seconds" << endl; }

    MPI_Finalize();
    return 0;
}

