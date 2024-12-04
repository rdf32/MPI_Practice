#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define NTHREADS 10


pthread_rwlock_t rwlock; // read write block

struct File {
    char data[100];
};

void* readData(void* arg){
    pthread_t thread_id = pthread_self();
    struct File* file = (struct File*) arg;

    pthread_rwlock_rdlock(&rwlock);
    printf("Thread reading: %ld -> %s\n", thread_id, file->data);
    pthread_rwlock_unlock(&rwlock);
    

};

void* writeData(void* arg){
    pthread_t thread_id = pthread_self();
    struct File* file = (struct File*) arg;

    pthread_rwlock_wrlock(&rwlock);
    printf("Thread %ld writing to the file\n", thread_id); 
    snprintf(file->data, sizeof(file->data), "Thread %ld wrote to this file", thread_id);
    pthread_rwlock_unlock(&rwlock);

};


int main(){
    clock_t start, end;
    double program_time;
    struct File sharedFile;
    pthread_t threads[NTHREADS];

    strcpy(sharedFile.data, "Hello world");
    start = clock();

    for(int i=0; i < NTHREADS; i++)
        {
            if (i % 2 == 0) {
                if(pthread_create(&threads[i], NULL, readData, &sharedFile)){
                        printf("Thread creation failed");
                    }
            }
            else{
                if(pthread_create(&threads[i], NULL, writeData, &sharedFile)){
                    printf("Thread creation failed");
                }
            }
        }

    for(int j=0; j < NTHREADS; j++)
        {
            pthread_join(threads[j], NULL); 
        }

    end = clock();
    program_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Execution Time: %f seconds\n", program_time);
    
    return 0;
}