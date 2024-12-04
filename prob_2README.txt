Question 2: Matrix-Matrix Multiplication Using Dynamic Load Balancing (50 Points) In this task, you will expand your 
implementation from Matrix-Vector multiplication (Question 1) to Matrix-Matrix multiplication using MPI. 
Each process will be responsible for computing a portion of the product of two large matrices 𝐴 and 𝐵, 
resulting in matrix 𝐶 = 𝐴 × 𝐵.

1. Instructions on how to compile your program.

- mpicc problem_1.c -o problem_1

2. Steps to run your program.

- mpirun - np <num_proc> -machinefile <machine_file.txt> <N1> <M1> <N2> <M2> <verbosity>
    - np: number of processes you want to run with 4, 8, 16, etc since this uses master / worker load balancing,
        the number of workers is np - 1
    - machinefile: file containg the name of your machines you want to use - this parameter is optional
    - N1: the number of rows you want in your matrix A and the number of rows in output matrix C
    - M1: number of columns you want in matrix A 
    - N2: the number of rows you want in matrix B
    - M2: number of columns in matrix B and number of columns in output matrix c

    - verbosity: set this to 1 if you want the matrix output to be printed to screen, set to 0 if you only want
        the run time to be printed to the screen
3. The average running time of your program.
{
    '1000x1000': 
    {
        '2': 6.312025, 
        '4': 2.380209, 
        '8': 3.028036, 
        '16': 5.697301
        },
 '5000x5000': 
    {
        '2': 991.103651,
        '4': 332.62674396,
        '8': 365.356369,
        '16': 481.875391
        },

 '10000x10000': 
    {
        '2': nan, # did not run this experiment
        '4': 3841.455735,
        '8': 4209.037501,
        '16': 4785.92305
        }
}

  Where NXM is the size of the matrix A and matrix B and 4, 8, 16 is the number of workers on DSU cluster
4. The expected output results when your program is run.
when verbosity is 1 you will get a print out of matrix A, vector b and result c along with time it took to run
when verbosity is 0 you get the time it took to run and number of processes that were used

5. M1 and N2 must be equivalent or this program (matrix multiplication) will not work
