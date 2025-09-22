#include "nqueens.h"

int main() 
{
    int n = 8;
    int* board = (int*)malloc(n * sizeof(int));
    if (board == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        board[i] = -1;
    }

    solveNQueensUtil(board, 0, n);

    for (int i = 0; i < n; i++)
        printf("%d:%d ", i, board[i]);
    printf("\n");

    free(board);
    return 0;
}
