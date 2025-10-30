#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000 // Size of the matrix
//#define NUM_THREADS 4  // Number of threads

int **A, **B, **C;  // Global matrices

// Structure to hold information for each thread
typedef struct 
{
    int thread_id;
    int num_rows;  // Number of rows each thread will handle
} thread_data_t;

// Function for each thread to perform matrix multiplication
void* matrixMultiplyThread(void* arg) 
{
    // Divide the task (rows) of each thread based on thread id
    // compute a portion of the matrix multiplication
    int thread_id = *(int*)arg;
    int start = thread_id * (N / NUM_THREADS);
    int end = (thread_id + 1) * (N / NUM_THREADS);

    for (int i = start; i < end; i++)
        for (int j = 0; j < N; j++) 
            for (int k = 0; k < N; k++) 
                C[i][j] += A[i][k] * B[k][j];

    return NULL;
}

void displayMatrix(int** matrix, int n) 
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
