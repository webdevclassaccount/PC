#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT 60*60*24
#define NUM_THREADS 4

typedef struct
{
    double x, y;
    double vx, vy;
    double mass;
} Body;

Body bodies[NUM_BODIES];

void compute_gravitational_force(Body *b1, Body *b2, double *fx, double *fy)
{
    double dx = b2->x - b1->x;
    double dy = b2->y - b1->y;
    double distance = sqrt(dx * dx + dy * dy);
    
    if (distance == 0.0)
    {
        *fx = *fy = 0.0;
        return;
    }
    
    double force_magnitude = G * b1->mass * b2->mass / (distance * distance);
    
    *fx = force_magnitude * dx / distance;
    *fy = force_magnitude * dy / distance;
}

void update_bodies(Body bodies[], int num_bodies, double dt)
{
    double fx, fy;
    
    #pragma omp parallel for private(fx, fy) num_threads(NUM_THREADS)
    for (int i = 0; i < num_bodies; i++)
    {
        fx = 0.0;
        fy = 0.0;
        
        for (int j = 0; j < num_bodies; j++)
        {
            if (i != j)
            {
                double force_x, force_y;
                compute_gravitational_force(&bodies[i], &bodies[j], 
                                           &force_x, &force_y);
                fx += force_x;
                fy += force_y;
            }
        }
        
        bodies[i].vx += fx / bodies[i].mass * dt;
        bodies[i].vy += fy / bodies[i].mass * dt;
    }
    
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < num_bodies; i++)
    {
        bodies[i].x += bodies[i].vx * dt;
        bodies[i].y += bodies[i].vy * dt;
    }
}

void print_positions(Body bodies[], int num_bodies)
{
    for (int i = 0; i < num_bodies; i++)
    {
        
    }
    
}

int main()
{
    double start_time, end_time;
    
    for (int i = 0; i < NUM_BODIES; i++)
    {
        bodies[i].x = rand() % 1000000000; 
        bodies[i].y = rand() % 1000000000;  
        bodies[i].vx = (rand() % 100 - 50) * 1e3; 
        bodies[i].vy = (rand() % 100 - 50) * 1e3; 
        bodies[i].mass = (rand() % 100 + 1) * 1e24; 
    }

    start_time = omp_get_wtime();
    
    for (int step = 0; step < 1000; step++)
    {
        
        print_positions(bodies, NUM_BODIES);
        update_bodies(bodies, NUM_BODIES, DT);
    }
    
    end_time = omp_get_wtime();
    
    printf("Time: %f\n", end_time - start_time);

    return 0;
}
