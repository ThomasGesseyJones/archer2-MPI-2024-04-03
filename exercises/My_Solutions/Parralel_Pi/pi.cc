#include "mpi.h"
#include <iostream>

using namespace std;

// Constants
int ACCURACY_N = 840;  // Number of terms in the series e.g., the accuracy of the calculation

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
    cout << "Hello from process " << rank << " of " << size << endl;

    // Calculate the partial sum attributed to this process
    // Check that the number of terms in the series is divisible by the number of processes
    if (ACCURACY_N % size != 0) {
        cerr << "The number of terms in the series must be divisible by the number of processes" << endl;
        return 1;
    }
    int terms_per_rank = ACCURACY_N / size;
    int sum_start = rank * terms_per_rank + 1;
    int sum_end = (rank + 1) * terms_per_rank;

    double part_sum = partial_sum(sum_start, sum_end, ACCURACY_N);



    //double pi = (4.0 / double(ACCURACY_N)) * part_sum;

    // Output
    cout << "Process " << rank << " partial sum " << part_sum << endl;
    return 0;
}
