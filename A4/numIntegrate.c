#include <stdio.h>

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

    for (int i = 1; i < N; i++) {
        x = limit1 + i * step;
        sum += f(x);
    }

    return sum * step;
}

int main() {
    double pi = trapezoidalRule();
    printf("Estimated value of π: %f\n", pi);
    return 0;
}
