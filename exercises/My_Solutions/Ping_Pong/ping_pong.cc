#include "mpi.h"
#include <iostream>

using namespace std;


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

    // List of analyses to perform
    int num_analyses = 10;
    int sizes[] = {300, 1000, 3000, 10000, 30000, 100000, 300000, 1000000, 3000000, 10000000};
    int iterations[] = {10000, 3000, 1000, 300, 100, 100, 100, 100, 100, 100};

    for (int analysis_id=0; analysis_id<num_analyses; analysis_id++) {
        // Start timing
        MPI_Barrier(comm); // Line up at the start line
        double tstart = MPI_Wtime(); // Fire the gun and start the clock

        // Pass array back and forth
        int num_times_to_exchange = iterations[analysis_id];
        int array_size = sizes[analysis_id];
        int array[array_size];
        for (int idx = 0; idx < array_size; idx++) { array[idx] = idx; }

        MPI_Status status;
        for (int exchange_idx = 0; exchange_idx < num_times_to_exchange; exchange_idx++) {
            if (rank == 0) {
                MPI_Ssend(array, array_size, MPI_INT, 1, 0, comm);
                MPI_Recv(array, array_size, MPI_INT, 1, 0, comm, &status);
            } else if (rank == 1) {
                MPI_Recv(array, array_size, MPI_INT, 0, 0, comm, &status);
                MPI_Ssend(array, array_size, MPI_INT, 0, 0, comm);
            }
        }

        // Finish timing
        MPI_Barrier(comm); // Wait for everyone to finish
        double tstop = MPI_Wtime(); // Stop the clock
        if (rank == 0) { cout << "Time taken: " << tstop - tstart << " seconds for analysis " << analysis_id << endl; }
    }

    MPI_Finalize();
    return 0;
}

