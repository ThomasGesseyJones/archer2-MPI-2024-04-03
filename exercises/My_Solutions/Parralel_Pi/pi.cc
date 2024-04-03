#include "mpi.h"
#include <iostream>

using namespace std;

// Constants
int ACCURACY_N = 840;  // Number of terms in the series e.g., the accuracy of the calculation
int NUM_TIMES_TO_COMPUTE_PI = 10000;  // Number of times to compute pi

double partial_sum(int start, int end, int total_n) {
    // Calculate the partial sum of 1/(1 + ((i-0.5)/total_n)^2) from i=start to i=end
    double total = 0.0;
    double n = double(total_n);

    double x;
    for (int i = start; i <= end; i++) {
        x = (double(i) - 0.5) / n;
        total += 1.0 / (1.0 + x*x);
    }

    return total;
}


int main() {
    // Aim is to calculate pi using a parallel algorithm

    // Initialize MPI
    MPI_Init(NULL, NULL);

    // Report in
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    cout.precision(10);  // Will want pi to many decimal places

    // Calculate the partial sum attributed to this process
    // Check that the number of terms in the series is divisible by the number of processes
    if (ACCURACY_N % size != 0) {
        cerr << "The number of terms in the series must be divisible by the number of processes" << endl;
        return 1;
    }
    int sum_start = (rank * ACCURACY_N)/size + 1;
    int sum_end = ((rank + 1) * ACCURACY_N)/size;
    cout << "Hello from process " << rank << " of " << size << " I will be summing ";
    cout << sum_start << " to " << sum_end << endl;

    // Start timing
    MPI_Barrier(MPI_COMM_WORLD); // Line up at the start line
    double tstart = MPI_Wtime(); // Fire the gun and start the clock
    double pi; // For end result

    // To get meaningful results, we will calculate pi NUM_TIMES_TO_COMPUTE_PI times
    for (int pi_calc_num = 0; pi_calc_num < NUM_TIMES_TO_COMPUTE_PI; pi_calc_num++) {
        // Partial sum for this process
        double part_sum = partial_sum(sum_start, sum_end, ACCURACY_N);
        //cout << "Process " << rank << " partial sum " << part_sum << endl;  // Output

        //Calculate the final sum
        double total_sum;
        double received_sum;
        MPI_Status status;
        if (rank == 0) {  // Rank 0 is to do the final sum
            total_sum = part_sum;
            for (int sender_rank = 1; sender_rank < size; sender_rank++) {
                MPI_Recv(&received_sum, 1, MPI_DOUBLE, sender_rank, 0, MPI_COMM_WORLD, &status);
                total_sum += received_sum;
            }

            // Calculate pi, printing to 10 decimal places
            pi = (4.0 / double(ACCURACY_N)) * total_sum;

        } else {
            MPI_Send(&part_sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    // Output the final pi
    if (rank == 0) {cout << "Final pi: " << pi << endl;}

    // Finish timing
    MPI_Barrier(MPI_COMM_WORLD); // Wait for everyone to finish
    double tstop = MPI_Wtime(); // Stop the clock
    if (rank == 0) {cout << "Time taken: " << tstop - tstart << " seconds" << endl;}

    MPI_Finalize();
    return 0;
}

