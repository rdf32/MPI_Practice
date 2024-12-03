Question 1:
A prime cluster is a set of three consecutive prime numbers that appear within a short interval. 
For example, {101, 103, 107} and {149, 151, 157} are clusters of three consecutive primes. 
Write a parallel program using Open MPI to search for and analyze prime clusters in a large range of integers.
Your program should find all prime clusters of size three within the range [2, 1000000]. The
range should be divided among processes so each one works on a separate sub-range

1. Instructions on how to compile your program.

- mpicc prime_clusters_mpi.c -o prime_clusters -lm

2. Steps to run your program.

- mpirun - np <num_proc> -machinefile <machine_file.txt> ./prime_clusters
    - np: number of processes you want to run with 4, 8, 16, etc since this uses master / worker load balancing,
        the number of workers is np - 1
    - machinefile: file containg the name of your machines you want to use - this parameter is optional

3. The average running time of your program.
    Workers	Time (s)	Speedup
    1	0.166	
    2	0.087	1.908
    4	0.064	2.593
    8	0.072	2.305


4. The expected output results when your program is run.
    Process 1 handling range [250001, 499999] 
    Process 2 handling range [500000, 749998]
    Process 3 handling range [749999, 1000000]
    Searching for prime clusters in range [2, 1000000] using 4 processes.
    Process 0 handling range [2, 250000]
    total number of primes: 78498 
    Total prime clusters found: 1360 
    Smallest-sum cluster: {5, 7, 11}
    Total processing time: 0.049747.

