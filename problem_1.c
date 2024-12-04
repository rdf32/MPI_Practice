#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


#define WORK 1
#define RESULT 2
#define BREAK 3

struct WorkerResult {
    double result;
    int row;
    int source;
};

struct Info {
    double* A;
    double* b;
    int N;
    int M;
    int verbose;
};

struct MasterInfo {
    double* c;
    int N;
};

int get_ntasks(struct Info* info){
    return info->N;
}

struct MasterInfo init_master(struct Info* info){
    struct MasterInfo master_info;
    master_info.c = (double*)malloc(info->N * sizeof(double));
    master_info.N = info->N;
    return master_info;
}

void decon_master(struct MasterInfo* master_info, struct Info* info){
    fflush(stdout);
    if  (info->verbose){
        printf("Result c:\n");
        for (int i = 0; i < master_info->N; i++) {
            printf("%f\n", master_info->c[i]);
        }
    }
    free(master_info->c); // release memory of result vector
}

struct Info load_info(int argc, char** argv, int rank){
    struct Info info;
    if (argc != 4) {
        if (rank == 0) {
            printf("Incorrect command arguments: %s <rows(N)> <columns(M)> <verbose>\n", argv[0]);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    info.N = atoi(argv[1]);
    info.M = atoi(argv[2]);
    info.verbose = atoi(argv[3]);
    info.A = NULL;
    info.b = NULL;
    return info;
}

void init_info(struct Info* info){
    info->A = (double*)malloc(info->N * info->M * sizeof(double));
    info->b = (double*)malloc(info->M * sizeof(double));
    
    if  (info->verbose){
        printf("Matrix A:\n");
    }
    for (int i = 0; i < info->N * info->M; i++) {
        info->A[i] = (double)(i + 1);
        if  (info->verbose){
            if ((i + 1)% info->M == 0){
                printf("%f \n", info->A[i]);
            } else {
                printf("%f ", info->A[i]);
            }
        }
    }
    if  (info->verbose){ 
        printf("\n");
    }
    if  (info->verbose){
        printf("Vector B:\n");
    }
    for (int i = 0; i < info->M; i++) {
        info->b[i] = (double)(i - 5);
        if  (info->verbose){
            printf("%f\n", info->b[i]);
        }
    }
}

void decon_info(struct Info* info){
    free(info->A);
    free(info->b);
}

void send_work(struct Info* info, int* nassigned, int worker){
    MPI_Send(nassigned, 1, MPI_INT, worker, WORK, MPI_COMM_WORLD); // send row to work on
    MPI_Send((info->A) + (*nassigned) * (info->M), info->M, MPI_DOUBLE, worker, WORK, MPI_COMM_WORLD); // send row data from A to worker
    MPI_Send(info->b, info->M, MPI_DOUBLE, worker, WORK, MPI_COMM_WORLD); // send vector b to worker
}

struct WorkerResult receive_result(){
    double result;
    MPI_Status status;
    struct WorkerResult worker_result;

    MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    worker_result.result = result;
    worker_result.row = status.MPI_TAG;
    worker_result.source = status.MPI_SOURCE;
    return worker_result;
}

void handle_result(struct MasterInfo* master, struct WorkerResult* wresult){
    master->c[wresult->row] = wresult->result;
}

void master(struct Info* info, int nworkers){

    int ntasks = get_ntasks(info);
    int ndone = 0;
    int nassigned = 0;
    printf("Matrix A size: %dX%d, vector b size: %d, output c size %d \n", 
        info->N, info->M, info->M, info->M);
    printf("number of processes %d \n", nworkers);

    struct MasterInfo master_info = init_master(info);

    // sent a row to each worker available to get started
    for (int worker = 1; worker < nworkers && nassigned < ntasks; worker++){
        send_work(info, &nassigned, worker);
        printf("Sending row %d to process %d \n", nassigned, worker);
        fflush(stdout);
        nassigned++;
    }


    // listen for results from worker, when receive a result send new work back to that worker if there is some left
    while (ndone < ntasks){ 
        struct WorkerResult result = receive_result();
        handle_result(&master_info, &result); // write row result to result vector
        ndone ++;

        if (nassigned < ntasks) {
            send_work(info, &nassigned, result.source); // send next bit of work to the worker who just sent a result
            printf("Sending row %d to process %d \n", nassigned, result.source);
            fflush(stdout);
            nassigned++;

        } else {
            int stop_signal = -1;
            MPI_Send(&stop_signal, 1, MPI_INT, result.source, BREAK, MPI_COMM_WORLD); // send work stop
        }
    }

    decon_master(&master_info, info); // release memory deconstruct the object
    
}

void worker(struct Info* info, int rank) {
    MPI_Status status;
    while (1){
        int rindx;
        MPI_Recv(&rindx, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // receive row index, so which row from A

        if (rindx == -1 || status.MPI_TAG == BREAK) { // -1 is sent as a breaking signal showing work is done
            break;
        }


        double* A_rindx = (double*)malloc(info->M * sizeof(double)); // allocate temp array to hold row from A sent from master
        double* b = (double*)malloc(info->M* sizeof(double)); // allocate temp array to hold row from b sent from master

        // now that I think about it, could probably just broadcast b to all workers up front and only send rows from A to speed things up

        MPI_Recv(A_rindx, info->M, MPI_DOUBLE, 0, WORK, MPI_COMM_WORLD, &status); // receive row from A to work on
        MPI_Recv(b, info->M, MPI_DOUBLE, 0, WORK, MPI_COMM_WORLD, &status); // receive b from master

        
        printf("Process %d handling row %d \n", rank, rindx);
        fflush(stdout);

        double res = 0.0;
        for (int i = 0; i < info->M; i++){
            res += A_rindx[i] * b[i]; // perform matrix multiplication
        }

        MPI_Send(&res, 1, MPI_DOUBLE, 0, rindx, MPI_COMM_WORLD); // send row result back to master

        free(A_rindx); // free worker allocated row
        free(b);
    }
}

int main(int argc, char** argv){
    
    double total_time; // variable for tracking program execution time

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = - MPI_Wtime();

    int nworkers;
    MPI_Comm_size(MPI_COMM_WORLD, &nworkers);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    struct Info info = load_info(argc, argv, rank);
    if (rank == 0){
        init_info(&info); // initialize info from command line into struct for latter use (attempt to generalize the code to have replaceable functions)
        master(&info, nworkers); // master function call
        decon_info(&info); // release any allocated memory in the info struct
        total_time += MPI_Wtime();
        printf("total processing time: %f. \n", total_time);

    } else {
        worker(&info, rank); // perform worker function
        //run worker process
    }
    
    MPI_Finalize();
}

