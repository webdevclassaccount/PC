#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

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

typedef struct
{
    Body *bodies;
    int num_bodies;
    int start_idx;
    int end_idx;
    double dt;
    pthread_barrier_t *barrier;
} ThreadArg;

Body bodies[NUM_BODIES];
pthread_barrier_t barrier;

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

void *update_bodies_thread(void *arg)
{
    ThreadArg *targ = (ThreadArg *)arg;
    double fx, fy;
    
    for (int i = targ->start_idx; i < targ->end_idx; i++)
    {
        fx = 0.0;
        fy = 0.0;
        
        for (int j = 0; j < targ->num_bodies; j++)
        {
            if (i != j)
            {
                double force_x, force_y;
                compute_gravitational_force(&targ->bodies[i], &targ->bodies[j], &force_x, &force_y);
                fx += force_x;
                fy += force_y;
            }
        }
        
        targ->bodies[i].vx += fx / targ->bodies[i].mass * targ->dt;
        targ->bodies[i].vy += fy / targ->bodies[i].mass * targ->dt;
    }
    
    pthread_barrier_wait(targ->barrier);
    
    for (int i = targ->start_idx; i < targ->end_idx; i++)
    {
        targ->bodies[i].x += targ->bodies[i].vx * targ->dt;
        targ->bodies[i].y += targ->bodies[i].vy * targ->dt;
    }
    
    pthread_barrier_wait(targ->barrier);
    
    return NULL;
}

void update_bodies(Body bodies[], int num_bodies, double dt)
{
    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];
    
    int bodies_per_thread = num_bodies / NUM_THREADS;
    int remainder = num_bodies % NUM_THREADS;
    
    int start = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i].bodies = bodies;
        thread_args[i].num_bodies = num_bodies;
        thread_args[i].start_idx = start;
        thread_args[i].end_idx = start + bodies_per_thread + (i < remainder ? 1 : 0);
        thread_args[i].dt = dt;
        thread_args[i].barrier = &barrier;
        
        pthread_create(&threads[i], NULL, update_bodies_thread, &thread_args[i]);
        
        start = thread_args[i].end_idx;
    }
    
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
}

int main()
{
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    
    for (int i = 0; i < NUM_BODIES; i++)
    {
        bodies[i].x = rand() % 1000000000; 
        bodies[i].y = rand() % 1000000000;  
        bodies[i].vx = (rand() % 100 - 50) * 1e3; 
        bodies[i].vy = (rand() % 100 - 50) * 1e3; 
        bodies[i].mass = (rand() % 100 + 1) * 1e24; 
    }

    for (int step = 0; step < 1000; step++)
        update_bodies(bodies, NUM_BODIES, DT);
    
    pthread_barrier_destroy(&barrier);

    return 0;
}
