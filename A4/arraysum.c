#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 100000000

long long sumArray(int arr[], long long size) 
{
    long long sum = 0;
    
    for (long long i = 0; i < size; i++)
    {
        sum += arr[i];
    }

    return sum;
}

int main() 
{
    omp_set_num_threads(NUM_THREADS);

    int *arr = malloc(SIZE * sizeof(int));

    double start_time = omp_get_wtime();
    #pragma omp parallel for
    for (long long i = 0; i < SIZE; i++)
        arr[i] = i + 1; 
    double end_time = omp_get_wtime();

    long long totalSum = sumArray(arr, SIZE);
    printf("Total Sum: %lld\n", totalSum);
    printf("Time taken : %lf\n", end_time - start_time);

    return 0;
}
