#include "OMP_Nqueens.h"

//#define NUM_THREADS 4

int main() 
{
    int n = 15;
    int* board = (int*)malloc(n * sizeof(int));
    if (board == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        board[i] = -1;
    }

    double start_time = omp_get_wtime();
    bool found = solveNQueensParallel(board, n, NUM_THREADS);
    double end_time = omp_get_wtime();

    for (int i = 0; i < n; i++)
        printf("%d:%d ", i, board[i]);
    printf("\n");    

    printf("Time taken : %lf\n", end_time - start_time);

    free(board);
    return 0;
}
