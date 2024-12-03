
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void merge(int* arr, int left, int mid, int right)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int L[n1];
    int R[n2];

    // Copy data to temp arrays L[] and R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    // Merge the temp arrays back into arr[l..r
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    // Copy the remaining elements of L[],
    // if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
    // Copy the remaining elements of R[],
    // if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// l is for left index and r is right index of the
// sub-array of arr to be sorted
void mergeSort(int* arr, int left, int right)
{
    if (left < right) {
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        #pragma omp task shared(arr) if(right - left > 10000)
        mergeSort(arr, left, mid);

        #pragma omp task shared(arr) if(right - left > 10000)
        mergeSort(arr, mid + 1, right);

        #pragma omp taskwait
        merge(arr, left, mid, right);
    }
}

void printArray(int* A, int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Incorrect command arguments: %s <array size> <nthreads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // omp_set_dynamic(0);              /** Explicitly disable dynamic teams **/
    omp_set_num_threads(atoi(argv[2]));
    double start, end;
    double cpu_time_used;

    int size = atoi(argv[1]);
    int *array = malloc(size * sizeof(int));
    if (array == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }

    // Fill the array with random numbers
    srand(42);
    for (int i = 0; i < size; i++) {
        array[i] = rand();
    }

    printf("Given array is \n");
    printArray(array, 10);

    start = omp_get_wtime();
    printf("Sorting array with size: %d \n", size);
    #pragma omp parallel
    {
        #pragma omp single
        mergeSort(array, 0, size - 1);
    }
    end = omp_get_wtime();

    printf("\nSorted array is \n");
    printArray(array, 10);
    free(array);

    cpu_time_used = ((double) (end - start));

    printf("Elapsed time: %f seconds\n", cpu_time_used);

    return 0;
}


// gcc mpmerge_sort.c -o mpmerge_sort -fopenmp
// ./mpmerge_sort 2000000 4