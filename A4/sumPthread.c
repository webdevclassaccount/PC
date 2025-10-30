#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 100000000
//#define NUM_THREADS 4

long long arr[SIZE];
long long partialSums[NUM_THREADS] = {0}; // Array to store partial sums for each thread

// Entry function for each thread
void* sumPart(void* arg) 
{
    // Divide the work for each thread based on their id and let them compute partial sums
        // -----> Write your code here
    int thread_id = *(int*)arg;
    int start = thread_id * (SIZE / NUM_THREADS); 
    int end = (thread_id + 1) * (SIZE / NUM_THREADS);  // Define range end
    long long count = 0;

    for (int i = start; i < end; i++)
        count += arr[i];

    partialSums[thread_id] = count;

    return NULL;
}

int main() 
{
    // Initialize the array
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1; 
    }

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Create threads to compute partial sums
        // ------> Write your code here
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, sumPart, &thread_ids[i]);
    }

    // Wait for all threads to finish
        // -------> Write your code here
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // Combine the partial sums from all threads
    long long totalSum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        totalSum += partialSums[i];
    }

    // Print the total sum;
    printf("Total Sum: %lld\n", totalSum);

    return 0;
}
