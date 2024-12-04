1. Instructions on how to compile your program.
2. Steps to run your program.
3. The average running time of your program.
4. The expected output results when your program is run.
5. Any additional information that will assist in compiling, running, and verifying your
answers.

Program: read_write.c
This file showcases how to use read and write mutex locks so that different threads can read from a FILE in parallel but 
only one thread can write to the FILE at a time. I simulated a file by creating a FILE struct that holds a string "data". 
I then create 10 threads, 5 are read threads and 5 are write threads and have them perform their respective operations. I utilized 
pthreads built in read and write mutex locks which handle the different concurrent operations. Multiple threads are allowed
to have read locks but only a single thread can have a write lock at one particular time. This ensures thread safety and makes
sure there are not race conditions. The type of synchronization I used was pthread create and pthread join so a 1 to 1 mapping 
between the main thread and each individual worker thread.

1. compile the program -> gcc -pthread read_write.c -o read_write
2. run the program -> ./read_write
3. the average running time of the program is 0.002 seconds -> I did not simulate any work by calling a sleep function on any of the threads
4. The expected outputs of the program are thread id, the operation they performed, and the data currently in the FILE
The writer threads print to the screen that they are writing to the FILE and they write to the FILE their thread id and that they wrote to it
The reader threads print to the screen their thread id, say they are reading the FILE and print to the screen the data they read
5. The data in the FILE should only say it was written to by a thread after the writer thread has released the write lock, This
can be verified in the program outputs by looking at the sequence of outputs to determine when write locks were held and when they weren't
by looking at what the reader threads say was currently held in the FILE->data