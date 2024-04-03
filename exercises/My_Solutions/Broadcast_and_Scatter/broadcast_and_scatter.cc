#include "mpi.h"
#include <iostream>

using namespace std;

// Constants
int ARRAY_SIZE = 12;  // Number of elements in the array

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
    for (int idx=0; idx < ARRAY_SIZE; idx++) {if (rank == 0) {array[idx] = idx;} else {array[idx] = -1;}}

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

    // Reset the arrays
    for (int idx=0; idx < ARRAY_SIZE; idx++) {if (rank == 0) {array[idx] = idx;} else {array[idx] = -1;}}

    // Manual scatter
    int communicated_array[ARRAY_SIZE / size];
    if (rank == 0){
        for (int receiver=1; receiver < size; receiver++){
            for (int idx=0; idx < ARRAY_SIZE / size; idx++) {
                communicated_array[idx] = array[(receiver * ARRAY_SIZE) / size + idx];
            }
            MPI_Ssend(communicated_array, ARRAY_SIZE / size, MPI_INT, receiver, 0, MPI_COMM_WORLD);
        }
    }
    else {
        MPI_Recv(communicated_array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int idx=0; idx < ARRAY_SIZE / size; idx++) {
            array[(rank * ARRAY_SIZE) / size + idx] = communicated_array[idx];
        }
    }

    // Print the array again
    print_array(rank, array, ARRAY_SIZE);

    MPI_Finalize();
    return 0;
}

