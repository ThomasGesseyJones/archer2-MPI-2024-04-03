#include "mpi.h"
#include <iostream>

using namespace std;

// Constants
ARRAY_SIZE = 12;  // Number of elements in the array

// Utility function to print an array
void print_array(int rank, int *array, int n)
{
    int i;
    printf("On rank %d, array[] = [", rank);
    for (i=0; i < n; i++)
    {
        if (i != 0) printf(",");
        printf(" %d", array[i]);
    }
    printf(" ]\n");
}


int main() {
    // Aim is to implement own equivalent to broadcast and scatter

    // Initialize MPI
    MPI_Init(NULL, NULL);

    // Report in
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    cout << "Hello from process " << rank << " of " << size << endl;

    // Initialize the array
    int array[ARRAY_SIZE];
    for (int idx=0; idx < ARRAY_SIZE; idx++) {if (rank == 0) {array[idx] = idx}; else {array[idx] = -1};}

    // Print the array
    print_array(rank, array, ARRAY_SIZE);

    // Manual broadcast
    if (rank == 0){
        for (int receiver=1; receiver < size; receiver++){
            MPI_Ssend(array, ARRAY_SIZE, MPI_INT, receiver, 0, MPI_COMM_WORLD);
        }
    }
    else {
        MPI_Recv(array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Print the array again
    print_array(rank, array, ARRAY_SIZE);

    // Start timing
    //MPI_Barrier(MPI_COMM_WORLD); // Line up at the start line
    //double tstart = MPI_Wtime(); // Fire the gun and start the clock

    // Finish timing
    //MPI_Barrier(MPI_COMM_WORLD); // Wait for everyone to finish
    //double tstop = MPI_Wtime(); // Stop the clock
    //if (rank == 0) {cout << "Time taken: " << tstop - tstart << " seconds" << endl;}

    MPI_Finalize();
    return 0;
}

