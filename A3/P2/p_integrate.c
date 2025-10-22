#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000000000 // intervals
//#define NUM_THREADS 1

double f(double x) {
    return 4.0 / (1.0 + x * x);
}

// use this shared global variable
double total_sum = 0.0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct {
    int thread_id;
    int num_threads;
    double a;
    double b;
    long n;
} ThreadData;

void *parallel_trapezoidalRule(void *arg) 
{
     ThreadData *data = (ThreadData *)arg;
    
    double h = (data->b - data->a) / data->n;
    long local_n = data->n / data->num_threads;
    long start = data->thread_id * local_n;
    long end = (data->thread_id == data->num_threads - 1) ? data->n : start + local_n;
    
    double local_sum = 0.0;
    double x;
    
    for (long i = start; i < end; i++) {
        x = data->a + i * h;
        if (i == 0 || i == data->n - 1) {
            local_sum += f(x) / 2.0;
        } else {
            local_sum += f(x);
        }
    }
    
    pthread_mutex_lock(&mutex);
    total_sum += local_sum;
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(NULL);
}  

int main(int argc, char *argv[]) 
{
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    double a = 0.0;
    double b = 1.0;
    double h = (b - a) / N;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = NUM_THREADS;
        thread_data[i].a = a;
        thread_data[i].b = b;
        thread_data[i].n = N;
        
        pthread_create(&threads[i], NULL, parallel_trapezoidalRule, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    total_sum *= h;
    
    printf("Result of numerical integration: %f\n", total_sum);
    
    pthread_mutex_destroy(&mutex);

    return 0;
}
