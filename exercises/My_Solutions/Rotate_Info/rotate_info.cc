#include "mpi.h"
#include <iostream>

using namespace std;


//Constants
const int NUM_LOOPS = 100000;

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

    // Setup-communication details
    int ahead = rank + 1;
    int behind = rank - 1;
    if (behind == -1) { behind = size - 1; }
    if (ahead == size) { ahead = 0; }
    MPI_Request send_request;
    MPI_Request recv_request;

    int total_so_far;
    int received;


    // Start timing
    MPI_Barrier(comm); // Line up at the start line
    double tstart = MPI_Wtime(); // Fire the gun and start the clock

    for (int loop_idx=0; loop_idx<NUM_LOOPS; loop_idx++) {
        // Start with value derived from tank
        int to_pass = (rank + 1) * (rank + 1); // Harder calculation
        total_so_far = to_pass;

        // Communication Loop
        for (int communication_step = 0; communication_step < size - 1; communication_step++) {
            // Non-blocking send to ahead and receive from behind
            MPI_Issend(&to_pass, 1, MPI_INT, ahead, 0, comm, &send_request);
            MPI_Irecv(&received, 1, MPI_INT, behind, 0, comm, &recv_request);

            // Waits for them all to finish
            MPI_Request requests[2] = {send_request, recv_request};
            MPI_Status statuses[2];
            MPI_Waitall(2, requests, statuses);

            // Update the total
            total_so_far += received;
            to_pass = received;
        }
    }

    // Print the total
    cout << "Process " << rank << " has total " << total_so_far << endl;

    // Finish timing
    MPI_Barrier(comm); // Wait for everyone to finish
    double tstop = MPI_Wtime(); // Stop the clock
    if (rank == 0) { cout << "Time taken: " << tstop - tstart << " seconds" << endl; }

    MPI_Finalize();
    return 0;
}

