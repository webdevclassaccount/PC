#include <stdio.h>
#include <stdlib.h> // For malloc() and free()
#include <unistd.h>
#include <omp.h>

#define N 1000 // Adjust this to test larger matrix sizes

void displayMatrix(int** matrix, int n) 
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void matrixMultiply(int** A, int** B, int** C, int n) 
{
    #pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++)
            {
                C[i][j] += (A[i][k] * B[k][j]);
            }
        }
    }
}
