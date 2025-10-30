#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>

bool isSafe(int board[], int row, int col, int n) 
{
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col)) {
            return false;
        }
    }
    return true;
}

void solveNQueensUtil(int board[], int col, int n) 
{
    if (col == n) return;
    
    for (int i = 0; i < n; i++) {
        if (isSafe(board, i, col, n)) {
            board[col] = i;
            solveNQueensUtil(board, col + 1, n); 
        }
    }
}

bool solveNQueensParallel(int board[], int n, int num_threads)
{
    bool found = false;
    
    #pragma omp parallel num_threads(num_threads) shared(found)
    {
        int* local_board = (int*)malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            local_board[i] = -1;
        }
        
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < n; i++) {
            if (found) continue;
            
            if (isSafe(local_board, i, 0, n)) {
                local_board[0] = i;
                solveNQueensUtil(local_board, 1, n);
                
                if (local_board[n-1] != -1) {
                    #pragma omp critical
                    {
                        if (!found) {
                            found = true;
                            for (int j = 0; j < n; j++) {
                                board[j] = local_board[j];
                            }
                        }
                    }
                }
            }
        }
        
        free(local_board);
    }
    
    return found;
}
