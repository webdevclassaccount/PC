#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

void parallelSieveOfEratosthenes(int n)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int limit = (int)sqrt(n);
    
    bool *base_primes = (bool *)malloc((limit + 1) * sizeof(bool));
    for (int i = 0; i <= limit; i++)
    {
        base_primes[i] = true;
    }
    base_primes[0] = base_primes[1] = false;

    for (int p = 2; p <= sqrt(limit); p++)
    {
        if (base_primes[p])
        {
            for (int i = p * p; i <= limit; i += p)
            {
                base_primes[i] = false;
            }
        }
    }

    int range_size = n - limit;
    int chunk_size = range_size / size;
    int remainder = range_size % size;
    
    int local_start = limit + 1 + rank * chunk_size + (rank < remainder ? rank : remainder);
    int local_end = local_start + chunk_size - 1 + (rank < remainder ? 1 : 0);
    
    if (local_end > n) local_end = n;
    int local_size = local_end - local_start + 1;

    bool *local_primes = NULL;
    if (local_size > 0)
    {
        local_primes = (bool *)malloc(local_size * sizeof(bool));
        for (int i = 0; i < local_size; i++)
        {
            local_primes[i] = true;
        }

        for (int p = 2; p <= limit; p++)
        {
            if (base_primes[p])
            {
                int start = ((local_start + p - 1) / p) * p;
                if (start < local_start) start += p;
                
                for (int i = start; i <= local_end; i += p)
                {
                    local_primes[i - local_start] = false;
                }
            }
        }
    }

    if (rank == 0)
    {
        //printf("Prime numbers up to %d are:\n", n);
        int total = 0;
        
        for (int i = 2; i <= limit; i++)
        {
            if (base_primes[i])
                total++;
        }
        
        if (local_size > 0)
        {
            for (int i = 0; i < local_size; i++)
                if (local_primes[i])
                    total++;
        }
        
        for (int proc = 1; proc < size; proc++)
        {
            int recv_start, recv_size;
            MPI_Recv(&recv_start, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&recv_size, 1, MPI_INT, proc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if (recv_size > 0)
            {
                bool *recv_primes = (bool *)malloc(recv_size * sizeof(bool));
                MPI_Recv(recv_primes, recv_size, MPI_C_BOOL, proc, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                for (int i = 0; i < recv_size; i++)
                {
                    if (recv_primes[i])
                        total++;
                        //printf("%d ", recv_start + i);
                }
                
                free(recv_primes);
            }
        }
        
        printf("Total Primes: %d\n", total);
    }
    else
    {
        MPI_Send(&local_start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&local_size, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        if (local_size > 0)
        {
            MPI_Send(local_primes, local_size, MPI_C_BOOL, 0, 2, MPI_COMM_WORLD);
        }
    }

    free(base_primes);
    if (local_size > 0)
    {
        free(local_primes);
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int n = 10000000;
    
    double start_time = MPI_Wtime();
    parallelSieveOfEratosthenes(n);
    double end_time = MPI_Wtime();
    
    if (rank == 0)
    {
        printf("Execution time: %.6f seconds\n", end_time - start_time);
    }
    
    MPI_Finalize();
    return 0;
}
