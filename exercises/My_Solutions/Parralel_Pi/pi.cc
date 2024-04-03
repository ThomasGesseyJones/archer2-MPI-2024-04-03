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

    // Calculate the partial sum
    double part_sum = partial_sum(1, ACCURACY_N + 1, ACCURACY_N);
    double pi = (4.0 / double(ACCURACY_N)) * part_sum;

    // Output
    cout << "Process " << rank << " calculated pi as " << pi << endl;
    return 0;
}

