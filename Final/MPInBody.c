#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT 60*60*24

typedef struct
{
    double x, y;
    double vx, vy;
    double mass;
} Body;

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

void update_bodies_parallel(Body local_bodies[], Body all_bodies[], int local_count, int local_offset, int total_bodies, double dt)
{
    double fx, fy;
    
    for (int i = 0; i < local_count; i++)
    {
        fx = 0.0;
        fy = 0.0;
        
        int global_i = local_offset + i;
        
        for (int j = 0; j < total_bodies; j++)
        {
            if (global_i != j)
            {
                double fx_temp, fy_temp;
                compute_gravitational_force(&local_bodies[i], &all_bodies[j], &fx_temp, &fy_temp);
                fx += fx_temp;
                fy += fy_temp;
            }
        }
        
        local_bodies[i].vx += fx / local_bodies[i].mass * dt;
        local_bodies[i].vy += fy / local_bodies[i].mass * dt;
    }
    
    for (int i = 0; i < local_count; i++)
    {
        local_bodies[i].x += local_bodies[i].vx * dt;
        local_bodies[i].y += local_bodies[i].vy * dt;
    }
}

int main(int argc, char *argv[])
{
    int rank, size;
    Body *bodies = NULL;
    Body *local_bodies = NULL;
    int local_count;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    MPI_Datatype MPI_BODY;
    int blocklengths[3] = {2, 2, 1};
    MPI_Aint displacements[3];
    MPI_Datatype types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
    
    displacements[0] = offsetof(Body, x);
    displacements[1] = offsetof(Body, vx);
    displacements[2] = offsetof(Body, mass);
    
    MPI_Type_create_struct(3, blocklengths, displacements, types, &MPI_BODY);
    MPI_Type_commit(&MPI_BODY);
    
    int base_count = NUM_BODIES / size;
    int remainder = NUM_BODIES % size;
    
    local_count = base_count + (rank < remainder ? 1 : 0);
    
    local_bodies = (Body *)malloc(local_count * sizeof(Body));
    
    if (rank == 0)
    {
        bodies = (Body *)malloc(NUM_BODIES * sizeof(Body));
        
        srand(42);
        
        for (int i = 0; i < NUM_BODIES; i++)
        {
            bodies[i].x = rand() % 1000000000; 
            bodies[i].y = rand() % 1000000000;  
            bodies[i].vx = (rand() % 100 - 50) * 1e3; 
            bodies[i].vy = (rand() % 100 - 50) * 1e3; 
            bodies[i].mass = (rand() % 100 + 1) * 1e24; 
        }
    }
    
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        sendcounts[i] = base_count + (i < remainder ? 1 : 0);
        displs[i] = offset;
        offset += sendcounts[i];
    }
    
    MPI_Scatterv(bodies, sendcounts, displs, MPI_BODY, local_bodies, local_count, MPI_BODY, 0, MPI_COMM_WORLD);
    
    Body *all_bodies = (Body *)malloc(NUM_BODIES * sizeof(Body));
    
    double start_time = MPI_Wtime();
    
    for (int step = 0; step < 1000; step++)
    {
        MPI_Allgatherv(local_bodies, local_count, MPI_BODY, all_bodies, sendcounts, displs, MPI_BODY, MPI_COMM_WORLD);
        
        update_bodies_parallel(local_bodies, all_bodies, local_count, displs[rank], NUM_BODIES, DT);
        
        //if (rank == 0 && step % 100 == 0)
            //printf("Completed step %d\n", step);
    }
    
    double end_time = MPI_Wtime();
    
    MPI_Gatherv(local_bodies, local_count, MPI_BODY, bodies, sendcounts, displs, MPI_BODY, 0, MPI_COMM_WORLD);
    
    if (rank == 0)
    {
        printf("time: %f\n", end_time - start_time);
        free(bodies);
    }
    
    free(sendcounts);
    free(displs);
    free(local_bodies);
    free(all_bodies);
    MPI_Type_free(&MPI_BODY);
    MPI_Finalize();
    
    return 0;
}
