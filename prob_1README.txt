Question 1: MPI Program for Matrix-Vector Multiplication with Dynamic Load Balancing (50 Points) Write an MPI program that
 performs Matrix-Vector Multiplication using dynamic load balancing. The matrix A (a large matrix of dimensions (𝑁 × 𝑀) 
 is distributed among multiple processes, and each process is responsible for multiplying a subset of blocks of rows 
 from the matrix by the vector 𝑏. The goal is to compute the result vector 𝑐 = 𝐴 × 𝑏

1. Instructions on how to compile your program.

- mpicc problem_1.c -o problem_1

2. Steps to run your program.

- mpirun - np <num_proc> -machinefile <machine_file.txt> <N> <M> <verbosity>
    - np: number of processes you want to run with 4, 8, 16, etc since this uses master / worker load balancing,
        the number of workers is np - 1
    - machinefile: file containg the name of your machines you want to use - this parameter is optional
    - N: the number of rows you want in your matrix A
    - M: number of columns you want in matrix A and the number of rows in vector b
    - verbosity: set this to 1 if you want the matrix output to be printed to screen, set to 0 if you only want
        the run time to be printed to the screen
3. The average running time of your program.
{
    '1000x1000': 
    {
        '2': 0.3795676666666667,
        '4': 0.16957,
        '8': 0.2793926666666667,
        '16': 0.5641366666666666
        },

    '5000x5000': 
    {
        '2': 6.527605333333334,
        '4': 1.7639623333333334,
        '8': 2.6129266666666666,
        '16': 4.194881333333334
        },

 '10000x10000': 
    {
        '2': 14.414980333333332,
        '4': 5.34763,
        '8': 19.402512,
        '16': 73.69554266666667
        }
}

  Where NXM is the size of the matrix A and 4, 8, 16 is the number of workers on DSU cluster
4. The expected output results when your program is run.
when verbosity is 1 you will get a print out of matrix A, vector b and result c along with time it took to run
when verbosity is 0 you get the time it took to run and number of processes that were used
