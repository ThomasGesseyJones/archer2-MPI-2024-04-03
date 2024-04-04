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

    // Setup-communication details
    int ahead = rank + 1;
    int behind = rank - 1;
    if (behind == -1) {behind = size-1;}
    if (ahead == size) {ahead = 0;}
    MPI_Request send_request;
    MPI_Request recv_request;
    MPI_Request requests[2] = {send_request, recv_request};
    MPI_Status statuses[2];

    int to_pass = rank;
    int total_so_far = to_pass;
    int received;

    // Communication Loop
    for (int communication_step=0; communication_step<size-1; communication_step++){
        // Non-blocking send to ahead and receive from behind
        MPI_Issend(&to_pass, 1, MPI_INT, ahead, 0, comm, &send_request);
        MPI_Irecv(&received, 1, MPI_INT, behind, 0, comm, &recv_request);

        // Waits for them all to finish
        MPI_Waitall(2, requests, statuses);

        // Update the total
        total_so_far += received;
        to_pass = received;
    }

    // Print the total
    cout << "Process " << rank << " has total " << total_so_far << endl;


    MPI_Finalize();
    return 0;
}

