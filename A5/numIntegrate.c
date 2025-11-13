#include <stdio.h>
#include <mpi.h>

#define N 1000000000 

double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

double trapezoidalRule(int rank, int size)
{
    double x;
    double limit1 = 0.0, limit2 = 1.0;
    double step;
    double local_sum, total_sum;
    int local_n;
    int remainder;
    int local_start;

    step = (limit2 - limit1) / N;
    local_n = N / size;
    remainder = N % size;

    if (rank < remainder)
    {
        local_n++;
        local_start = rank * local_n;
    }
    else
    {
        local_start = rank * local_n + remainder;
    }

    local_sum = 0.0;

    if (rank == 0)
    {
        local_sum += 0.5 * f(limit1);
    }

    for (int i = 0; i < local_n; i++)
    {
        int global_i = local_start + i;
        if (global_i > 0 && global_i < N)
        {
            x = limit1 + global_i * step;
            local_sum += f(x);
        }
    }

    if (rank == size - 1)
    {
        local_sum += 0.5 * f(limit2);
    }

    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    return total_sum * step;
}

int main(int argc, char** argv)
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = MPI_Wtime();
    trapezoidalRule(rank, size);
    double end_time = MPI_Wtime();

    if (rank == 0)
    {
        printf("Execution time: %.6f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
