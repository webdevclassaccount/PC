// Parallelize N queens with pthreads!

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define N 8 
#define K 2 
//#define MAX_THREADS 8 

typedef struct {
    int board[N];
    int start_col;
    int solutions;
} WorkItem;

WorkItem *work_queue;
int work_count = 0;
int work_index = 0;
pthread_mutex_t work_mutex;

bool isSafe(int board[], int row, int col) 
{
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == col - i)
            return false;
    }
    return true;
}

int solve(int board[], int col) 
{
    if (col == N) {
        return 1;
    }

    int solutions = 0;
    for (int row = 0; row < N; row++) {
        if (isSafe(board, row, col)) {
            board[col] = row;
            solutions += solve(board, col + 1);
        }
    }
    return solutions;
}

void generateWork(int board[], int col, int k)
{
    if (col == k) {
        memcpy(work_queue[work_count].board, board, N * sizeof(int));
        work_queue[work_count].start_col = k;
        work_queue[work_count].solutions = 0;
        work_count++;
        return;
    }

    for (int row = 0; row < N; row++) {
        if (isSafe(board, row, col)) {
            board[col] = row;
            generateWork(board, col + 1, k);
        }
    }
}

void* worker(void* arg)
{
    while (true) {
        pthread_mutex_lock(&work_mutex);
        if (work_index >= work_count) {
            pthread_mutex_unlock(&work_mutex);
            break;
        }
        int my_index = work_index++;
        pthread_mutex_unlock(&work_mutex);

        int local_board[N];
        memcpy(local_board, work_queue[my_index].board, N * sizeof(int));
        work_queue[my_index].solutions = solve(local_board, work_queue[my_index].start_col);
    }
    return NULL;
}

int main() 
{
    int board[N];

    for (int i = 0; i < N; i++)
        board[i] = -1;

    int max_work = 1;
    for (int i = 0; i < K; i++)
        max_work *= N;
    work_queue = (WorkItem*)malloc(max_work * sizeof(WorkItem));

    generateWork(board, 0, K);

    pthread_t threads[MAX_THREADS];
    int num_threads = (work_count < MAX_THREADS) ? work_count : MAX_THREADS;
    
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    int total_solutions = 0;
    for (int i = 0; i < work_count; i++) {
        total_solutions += work_queue[i].solutions;
    }

    printf("Total solutions for N=%d: %d\n", N, total_solutions);

    free(work_queue);
    pthread_mutex_destroy(&work_mutex);

    return 0;
}
