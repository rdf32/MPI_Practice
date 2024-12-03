#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>

#define LOWER 2            // Starting integer of the range
#define UPPER 1000000      // Ending integer of the range
#define CHUNK_SIZE 10000   // Size of the chunk each process will handle

// Optimized Trial Division Method - O(√n) time and O(1) space
// Function to check whether a number is prime or not
int is_prime(int n) {
    // Check if n is 1 or 0
    if (n <= 1) return 0;
    if (n == 2 || n == 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;

    for (int i = 5; i <= sqrt(n); i = i + 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;

    return 1;
}

int main(int argc, char** argv){
    int rank, workers;
    double total_time;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = - MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &workers);

    if (rank == 0){
        printf("Searching for prime clusters in range [2, 1000000] using %d processes. \n", workers);
        fflush(stdout);
    }
    int total_primes;
    int *buffer = NULL; 
    int capacity = 10;
    int nprimes = 0;
    // Divide the range into subranges for each process
    int range_per_process = (UPPER - LOWER + 1) / workers;
    int start = LOWER + rank * range_per_process;
    int end = (rank == workers - 1) ? UPPER : start + range_per_process - 1;
    // Ensure the range is not empty
    if (start > end) {
        MPI_Finalize();
        return 0;
    }

    printf("Process %d handling range [%d, %d] \n", rank, start, end);

    buffer = (int *)malloc(capacity * sizeof(int));
    if (buffer == NULL) {
        printf("Memory allocation failed!\n");
        MPI_Finalize();
        return -1;
    }

    for (int num = start; num <= end; num++) {
        if (is_prime(num)) {
            // Resize the array if it's full
            if (nprimes == capacity) {
                capacity *= 2;  // Double the capacity
                int *temp = (int *)realloc(buffer, capacity * sizeof(int));
                if (temp == NULL) {
                    printf("Memory reallocation failed\n");
                    free(buffer);
                    MPI_Finalize();
                    return -1; 
                }
                buffer = temp;  // Assign the new buffer back only if realloc succeeded
            }

            // Add the new value to the array
            buffer[nprimes] = num;
            nprimes++;
        }
        
    }
    // printf("Rank %d found %d primes \n", rank, nprimes);
    MPI_Reduce(&nprimes, &total_primes, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank != 0){
        MPI_Send(&nprimes, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Send(buffer, nprimes, MPI_INT, 0, 2, MPI_COMM_WORLD);
        // send primes
        free(buffer);
    }
    
    if (rank == 0){
        printf("total number of primes: %d \n", total_primes);
        // allocate memory for primes
        int nclusters = 0;
        int p_rec = 0;
        int* primes = (int *)malloc(total_primes * sizeof(int));

        for (int i=0; i < nprimes; i++){
            primes[p_rec] = buffer[i];
            p_rec++;
        }
        free(buffer);

        for (int worker = 1; worker < workers; worker++){
            int wnprimes;
            MPI_Status status;
            MPI_Recv(&wnprimes, 1, MPI_INT, worker, 1, MPI_COMM_WORLD, &status);
            int* worker_primes = (int *)malloc(wnprimes * sizeof(int));
            MPI_Recv(worker_primes, wnprimes, MPI_INT, worker, 2, MPI_COMM_WORLD, &status);

            for (int i=0; i < wnprimes; i++){
                primes[p_rec] = worker_primes[i];
                p_rec ++;
            }
            free(worker_primes);
        }
            
        // Loop through the list of primes, checking for consecutive primes
        float min_cluster_sum = INFINITY;
        int min_cluster[3] = {0, 0, 0};
        for (int i = 0; i < total_primes - 2; i++) {
            // Check if there are three consecutive primes
            if (primes[i + 1] - primes[i] == 2 && primes[i + 2] - primes[i + 1] == 4) {
                int cluster_sum = primes[i] + primes[i + 1] + primes[i + 2];
                if ((float)cluster_sum < min_cluster_sum){
                    min_cluster_sum = (float)cluster_sum;
                    min_cluster[0] = primes[i];
                    min_cluster[1] = primes[i + 1];
                    min_cluster[2] = primes[i + 2];

                }
                i += 2;
                nclusters++;
            }
        }
        printf("Total prime clusters found: %d \n", nclusters);
        printf("Smallest-sum cluster: {%d, %d, %d} \n", min_cluster[0], min_cluster[1], min_cluster[2]);
        total_time += MPI_Wtime();
        printf("Total processing time: %f. \n", total_time);

        free(primes);

    }

    MPI_Finalize();
    return 0;
}
//  mpicc prime_clusters.c -o prime_clusters -lm
// mpirun -np 4 prime_clusters