#include <stdio.h>
#include <unistd.h>
#include <omp.h>

#define N 1000000000// intervals

double f(double x) {
    return 4.0 / (1.0 + x * x); // Function to integrate
}

double trapezoidalRule() 
{
    double x;
    double limit1 = 0.0, limit2 = 1.0;
    double step;
    double sum;

    sum = 0.5 * (f(limit1) + f(limit2));
    step = (limit2 - limit1) / N;
    
    #pragma omp parallel for private(x) reduction(+:sum)
    for (int i = 1; i < N; i++) {
        x = limit1 + i * step;
        sum += f(x);
    }

    return sum * step;
}

int main() {
    omp_set_num_threads(NUM_THREADS);

    double start_time = omp_get_wtime();
    double pi = trapezoidalRule();
    double end_time = omp_get_wtime();

    printf("Estimated value of π: %f\n", pi);
    printf("Time taken : %lf\n", end_time - start_time);
    return 0;
}
