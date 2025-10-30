#include "matrixT.h"

int main() 
{
    // declare thread id and thread data
    
    // Dynamically allocate memory for the matrices
    A = (int**)malloc(N * sizeof(int*));
    B = (int**)malloc(N * sizeof(int*));
    C = (int**)malloc(N * sizeof(int*));

    for (int i = 0; i < N; ++i) {
        A[i] = (int*)malloc(N * sizeof(int));
        B[i] = (int*)malloc(N * sizeof(int));
        C[i] = (int*)malloc(N * sizeof(int));
    }

    if (A == NULL || B == NULL || C == NULL) {
        //printf("Memory allocation failed!\n");
        return -1;
    }

    // Initialize matrices A and B with values
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 0;
        }
    }

    //printf("Matrices initialized successfully.\n");

    // Create threads to perform matrix multiplication
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, matrixMultiplyThread, &thread_ids[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);


    printf("Matrix multiplication complete!\n");

    // Optionally, display the resulting matrix C (Not when you are timing :) )
    //displayMatrix(C, N);

    // Free dynamically allocated memory
    for (int i = 0; i < N; ++i) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    return 0;
}
