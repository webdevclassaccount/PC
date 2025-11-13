#include "matrixMul.h"

int main(int argc, char** argv) 
{
    int rank, size;
    int** A = NULL;
    int** B = NULL;
    int** C = NULL;
    int** local_A = NULL;
    int** local_C = NULL;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int rows_per_proc = N / size;
    int remainder = N % size;
    
    B = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i)
    {
        B[i] = (int*)malloc(N * sizeof(int));
    }
    
    local_A = (int**)malloc(rows_per_proc * sizeof(int*));
    local_C = (int**)malloc(rows_per_proc * sizeof(int*));
    for (int i = 0; i < rows_per_proc; ++i)
    {
        local_A[i] = (int*)malloc(N * sizeof(int));
        local_C[i] = (int*)malloc(N * sizeof(int));
    }
    
    if (rank == 0)
    {
        A = (int**)malloc(N * sizeof(int*));
        C = (int**)malloc(N * sizeof(int*));
        
        for (int i = 0; i < N; ++i)
        {
            A[i] = (int*)malloc(N * sizeof(int));
            C[i] = (int*)malloc(N * sizeof(int));
        }
        
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                A[i][j] = 1;
                B[i][j] = 1;
                C[i][j] = 0;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
    {
        MPI_Bcast(B[i], N, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    if (rank == 0)
    {
        for (int i = 0; i < rows_per_proc; i++)
        {
            for (int j = 0; j < N; j++)
            {
                local_A[i][j] = A[i][j];
            }
        }
        
        for (int proc = 1; proc < size; proc++)
        {
            for (int i = 0; i < rows_per_proc; i++)
            {
                MPI_Send(A[proc * rows_per_proc + i], N, MPI_INT, proc, 0, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        for (int i = 0; i < rows_per_proc; i++)
        {
            MPI_Recv(local_A[i], N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    
    double start_time = MPI_Wtime();
    matrixMultiply(local_A, B, local_C, rows_per_proc);
    double end_time = MPI_Wtime();
    
    if (rank == 0)
    {
        for (int i = 0; i < rows_per_proc; i++)
        {
            for (int j = 0; j < N; j++)
            {
                C[i][j] = local_C[i][j];
            }
        }
        
        for (int proc = 1; proc < size; proc++)
        {
            for (int i = 0; i < rows_per_proc; i++)
            {
                MPI_Recv(C[proc * rows_per_proc + i], N, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        
        printf("Execution time: %.6f seconds\n", end_time - start_time);
    }
    else
    {
        for (int i = 0; i < rows_per_proc; i++)
        {
            MPI_Send(local_C[i], N, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    
    for (int i = 0; i < N; ++i)
    {
        free(B[i]);
    }
    free(B);
    
    for (int i = 0; i < rows_per_proc; ++i)
    {
        free(local_A[i]);
        free(local_C[i]);
    }
    free(local_A);
    free(local_C);
    
    if (rank == 0)
    {
        for (int i = 0; i < N; ++i)
        {
            free(A[i]);
            free(C[i]);
        }
        free(A);
        free(C);
    }
    
    MPI_Finalize();
    return 0;
}
