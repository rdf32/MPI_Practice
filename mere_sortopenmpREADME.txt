Question 1:
A prime cluster is a set of three consecutive prime numbers that appear within a short interval. 
For example, {101, 103, 107} and {149, 151, 157} are clusters of three consecutive primes. 
Write a parallel program using Open MPI to search for and analyze prime clusters in a large range of integers.
Your program should find all prime clusters of size three within the range [2, 1000000]. The
range should be divided among processes so each one works on a separate sub-range

1. Instructions on how to compile your program.

-  gcc merge_sort_openmp.c -o merge_sort_openmp -fopenmp

2. Steps to run your program.

- ./merge_sort_sequential <size of arrray> <number of threads>
- ./merge_sort_openmp 2000000 8


3. The average running time of your program.
    Threads	Sequential Time (s)	OpenMP Time (s)	Speedup
    1	0.510990	0.570504	0.89568
    2	0.510990	0.272888	1.87252
    4	0.510990	0.257227	1.98653
    8	0.510990	0.182599	2.79842




4. The expected output results when your program is run.
    Given array is 
    71876166 708592740 1483128881 907283241 442951012 537146758 1366999021 1854614940 647800535 53523743
    Sorting array with size: 2000000

    Sorted array is
    2275 2792 3285 3597 3612 3889 4599 4791 6573 6782
    Elapsed time: 0.234430 seconds

