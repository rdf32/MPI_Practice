#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define WORK 1
#define RESULT 2
#define BREAK 3
#define CHUNK_SIZE 3

struct Chunk {
    int nrows;
    int ncols;
    int row_start;
    int col_start;
};

struct Info {
    int N1;
    int M1;
    int N2;
    int M2;
    int verbose;
    int chunk_size;
};

struct Info load_info(int argc, char** argv, int chunk_size){
    struct Info info;

    info.N1 = atoi(argv[1]);
    info.M1 = atoi(argv[2]);
    info.N2 = atoi(argv[3]);
    info.M2 = atoi(argv[4]);
    info.verbose = atoi(argv[5]);
    info.chunk_size = chunk_size;

    return info;
}

void displayProgressBar(int progress, int total) {
    int barWidth = 50; // Width of the progress bar
    int pos = (progress * barWidth) / total;

    printf("[");
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            printf("#");
        else
            printf(" ");
    }
    printf("] %d%%\r", (progress * 100) / total);
    fflush(stdout); // Force the output to be printed
}

struct Chunk* get_tasks(struct Info* info, int* total_chunks) {
    int nrow_chunks = info->N1 / info->chunk_size;
    int ncol_chunks = info->M2 / info->chunk_size;

    // Check if there are remaining rows/columns that don't fit evenly into chunks
    int remainder_rows = info->N1 % info->chunk_size;
    int remainder_cols = info->M2 % info->chunk_size;

    // If there are remainder rows, add one extra row chunk
    if (remainder_rows != 0) {
        nrow_chunks += 1;
    }

    // If there are remainder columns, add one extra column chunk
    if (remainder_cols != 0) {
        ncol_chunks += 1;
    }

    // Total number of chunks
    *total_chunks = nrow_chunks * ncol_chunks;

    // Allocate memory for the chunks
    struct Chunk* chunks = (struct Chunk*) malloc((*total_chunks) * sizeof(struct Chunk));

    // Fill in the chunk sizes and row/column offsets
    for (int i = 0; i < nrow_chunks; ++i) {
        for (int j = 0; j < ncol_chunks; ++j) {
            // Calculate the number of rows and columns for this chunk
            int chunk_nrows = (i == nrow_chunks - 1 && remainder_rows != 0) ? remainder_rows : info->chunk_size;
            int chunk_ncols = (j == ncol_chunks - 1 && remainder_cols != 0) ? remainder_cols : info->chunk_size;

            // Calculate the starting row and column for this chunk in the original matrices
            int row_start = i * info->chunk_size;
            int col_start = j * info->chunk_size;

            // Store the chunk size and the starting row/column in the array
            chunks[i * ncol_chunks + j].nrows = chunk_nrows;
            chunks[i * ncol_chunks + j].ncols = chunk_ncols;
            chunks[i * ncol_chunks + j].row_start = row_start;
            chunks[i * ncol_chunks + j].col_start = col_start;
        }
    }

    return chunks;
}

int main(int argc, char** argv){

    double total_time;

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = - MPI_Wtime();

    int nworkers;
    MPI_Comm_size(MPI_COMM_WORLD, &nworkers);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (argc != 6) {
        if (rank == 0) {
            printf("Incorrect command arguments: %s <rows(N1)> <columns(M1)>  <rows(N2)> <columns(M2)> <verbose(1)>\n", argv[0]);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    struct Info info = load_info(argc, argv, CHUNK_SIZE);

    

    if (rank == 0){
        printf("Matrix A size: %dX%d, Matrix B size: %dx%d, output C size %dx%d \n", 
        info.N1, info.M1, info.N2, info.M2, info.N1, info.M2);
        printf("number of processes %d \n", nworkers);

        double* A = (double*)malloc(info.N1 * info.M1 * sizeof(double));
        double* B = (double*)malloc(info.N2 * info.M2 * sizeof(double));
        double* C = (double*)malloc(info.N1 * info.M2 * sizeof(double));

        // break A and B into row and col chunks based on chunk size
        // get total number of chunks based on size of A and B, create vector of chunks to be sent to workers
        int total_chunks;
        struct Chunk* chunks = get_tasks(&info, &total_chunks);
        printf("number of tasks %d \n", total_chunks);


        if (info.verbose){
            printf("Matrix A:\n");
        }
        // initialize A
        for (int i = 0; i < info.N1 * info.M1; i++) {
            A[i] = (double)(i + 1);
            if (info.verbose){
                if ((i + 1)% info.M1 == 0){
                    printf("%f \n", A[i]);
                } else {
                    printf("%f ", A[i]);
                }
            }

        }
        if (info.verbose){
            printf("Matrix B:\n");
        }
        // initialize B

        for (int i = 0; i < info.N2 * info.M2; i++) {
            B[i] = (double)(i + 2);
            if (info.verbose){
                if ((i + 1) % info.M2 == 0){
                    printf("%f \n", B[i]);
                } else {
                    printf("%f ", B[i]);
                }
            }
        }
        int ndone = 0;
        int nassigned = 0;
        for (int worker = 1; worker < nworkers; worker++) {
            struct Chunk chunk = chunks[nassigned];
            double* A_sub = A + (chunk.row_start * info.M1);
            double* B_sub = (double*)malloc(info.N2 * chunk.ncols * sizeof(double));

            // pull the chunk size columns from B since B is stored as a flat vector with row major spacing
            for (int i = 0; i < info.N2; i++) {
                for (int j = 0; j < chunk.ncols; j++) {
                    // Calculate the index in the flat array for matrix B
                    B_sub[i * chunk.ncols + j] = B[i * info.M2 + (chunk.col_start + j)];
                }
            }

            MPI_Send(&chunk.row_start, 1, MPI_INT, worker, WORK, MPI_COMM_WORLD); // send starting row index
            MPI_Send(&chunk.col_start, 1, MPI_INT, worker, WORK, MPI_COMM_WORLD); // send starting column index
            MPI_Send(&chunk.nrows, 1, MPI_INT, worker, WORK, MPI_COMM_WORLD);      // send number of rows in this chunk
            MPI_Send(&chunk.ncols, 1, MPI_INT, worker, WORK, MPI_COMM_WORLD);
            MPI_Send(A_sub, chunk.nrows * info.M1, MPI_DOUBLE, worker, WORK, MPI_COMM_WORLD); // send chunk size rows
            MPI_Send(B_sub, info.N2 * chunk.ncols, MPI_DOUBLE, worker, WORK, MPI_COMM_WORLD); // send chunk size columns
            printf("Sending %d rows and %d cols to process %d \n", chunk.nrows, chunk.ncols, worker);
            fflush(stdout);
            free(B_sub);
            nassigned++;
            
        }
        // continually listen for results from workers
        while (ndone < total_chunks){
            MPI_Status status;
            int row_start;
            MPI_Recv(&row_start, 1, MPI_INT, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, &status);

            int source = status.MPI_SOURCE; // once you get a result only receieve the rest of the data from that source
            // this is important for preventing race conditions
            // all receive below this are forced to come from the same source

            int col_start;
            MPI_Recv(&col_start, 1, MPI_INT, source, RESULT, MPI_COMM_WORLD, &status);
            int nrows;
            MPI_Recv(&nrows, 1, MPI_INT, source, RESULT, MPI_COMM_WORLD, &status);
            int ncols;
            MPI_Recv(&ncols, 1, MPI_INT, source, RESULT, MPI_COMM_WORLD, &status);


            // Allocate memory for the result chunk
            double* C_sub = (double*)malloc(nrows * ncols * sizeof(double));
            MPI_Recv(C_sub, nrows * ncols, MPI_DOUBLE, source, RESULT, MPI_COMM_WORLD, &status);
            
            
            for (int i = 0; i < nrows; i++) {
                for (int j = 0; j < ncols; j++) {
                    C[(row_start + i) * info.M2 + (col_start + j)] = C_sub[i * ncols + j];
                }
            }
            free(C_sub);
            // displayProgressBar(ndone, total_chunks);
            ndone ++;
            //  send more work back to the same worker that just send a result
            if (nassigned < total_chunks) {
                struct Chunk chunk = chunks[nassigned];
                double* A_sub = A + (chunk.row_start * info.M1);
                double* B_sub = (double*)malloc(info.N2 * chunk.ncols * sizeof(double));

                for (int i = 0; i < info.N2; i++) {
                    for (int j = 0; j < chunk.ncols; j++) {
                        // Calculate the index in the flat array for matrix B
                        B_sub[i * chunk.ncols + j] = B[i * info.M2 + (chunk.col_start + j)];
                    }
                }

                MPI_Send(&chunk.row_start, 1, MPI_INT, source, WORK, MPI_COMM_WORLD); // send starting row index
                MPI_Send(&chunk.col_start, 1, MPI_INT, source, WORK, MPI_COMM_WORLD); // send starting column index
                MPI_Send(&chunk.nrows, 1, MPI_INT, source, WORK, MPI_COMM_WORLD);      // send number of rows in this chunk
                MPI_Send(&chunk.ncols, 1, MPI_INT, source, WORK, MPI_COMM_WORLD);
                MPI_Send(A_sub, chunk.nrows * info.M1, MPI_DOUBLE, source, WORK, MPI_COMM_WORLD);
                MPI_Send(B_sub, info.N2 * chunk.ncols, MPI_DOUBLE, source, WORK, MPI_COMM_WORLD);
                printf("Sending %d rows and %d cols to process %d \n", chunk.nrows, chunk.ncols, source);
                fflush(stdout);
                free(B_sub);
                nassigned++;

            } else {
                int stop_signal = -1;
                MPI_Send(&stop_signal, 1, MPI_INT, source, BREAK, MPI_COMM_WORLD);
            }
        }
        // print result matrix C
        if (info.verbose){
            fflush(stdout);
            printf("Matrix C:\n");
            for (int i = 0; i < info.N1 * info.M2; i++) {
                if ((i + 1)% info.M2 == 0){
                    printf("%f \n", C[i]);
                } else {
                    printf("%f ", C[i]);
                }
            }
        }

        total_time += MPI_Wtime();
        printf("total processing time: %f. \n", total_time);

        free(A);
        free(B);
        free(C);
        free(chunks);

            
    } else{
        while(1){
            MPI_Status status;

            int row_start;
            MPI_Recv(&row_start, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (row_start == -1 || status.MPI_TAG == BREAK) {
                break;
            }
            int col_start;
            MPI_Recv(&col_start, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);
            int nrows;
            MPI_Recv(&nrows, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);
            int ncols;
            MPI_Recv(&ncols, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);

            int source = status.MPI_SOURCE;
            printf("Process %d handling  %d rows and %d cols \n", rank, nrows, ncols);
            fflush(stdout);
            double* A_sub = (double*)malloc(nrows * info.M1 * sizeof(double)); // Submatrix from A
            double* B_sub = (double*)malloc(info.N2 * ncols * sizeof(double));   // Submatrix from B
            double* C_sub = (double*)malloc(nrows * ncols * sizeof(double));

            MPI_Recv(A_sub, nrows * info.M1, MPI_DOUBLE, 0, WORK, MPI_COMM_WORLD, &status);
            MPI_Recv(B_sub, info.M2 * ncols, MPI_DOUBLE, 0, WORK, MPI_COMM_WORLD, &status);

            for (int i = 0; i < nrows; i++) {
                for (int j = 0; j < ncols; j++) {
                    C_sub[i * ncols + j] = 0.0; // Initialize the result for the current element
                    for (int k = 0; k < info.M1; k++) {
                        C_sub[i * ncols + j] += A_sub[i * info.M1 + k] * B_sub[k * ncols + j];
                    }
                }
            }
            MPI_Send(&row_start, 1, MPI_INT, 0, RESULT, MPI_COMM_WORLD); // send starting row index
            MPI_Send(&col_start, 1, MPI_INT, 0, RESULT, MPI_COMM_WORLD); // send starting column index
            MPI_Send(&nrows, 1, MPI_INT, 0, RESULT, MPI_COMM_WORLD);      // send number of rows in this chunk
            MPI_Send(&ncols, 1, MPI_INT, 0, RESULT, MPI_COMM_WORLD);      // send number of columns in this chunk
            MPI_Send(C_sub, nrows * ncols, MPI_DOUBLE, 0, RESULT, MPI_COMM_WORLD);

            free(A_sub);
            free(B_sub);
            free(C_sub);
        }
    }
    MPI_Finalize();
}

// How They Work Together
// ppr:4:node determines how many MPI processes run per machine.
// PE=4 then specifies how many cores each of those processes should use.
// So, using both ppr:4:node and PE=4 would mean:
// mpirun --map-by ppr:4:node:PE=4 ./your_program:
// Launching 4 MPI processes on each node.
// Assigning 4 cores to each MPI process.
// Example Scenario
// Consider a setup with 4 nodes, each with 16 cores (4 processors, 4 cores each). Here’s what happens with 