#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <CL/cl.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT 60*60*24
#define MAX_SOURCE_SIZE (0x100000)

typedef struct
{
    double x, y;
    double vx, vy;
    double mass;
} Body;

double wtime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

char* load_kernel_source(const char *filename)
{
    FILE *fp;
    char *source_str;
    size_t source_size;
    
    fp = fopen(filename, "r");
    if (!fp)
        exit(1);
    
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    source_str[source_size] = '\0';
    fclose(fp);
    
    return source_str;
}

int main()
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel_forces, kernel_velocities, kernel_positions;
    
    Body *bodies = (Body *)malloc(NUM_BODIES * sizeof(Body));
    double *fx = (double *)malloc(NUM_BODIES * sizeof(double));
    double *fy = (double *)malloc(NUM_BODIES * sizeof(double));
    
    for (int i = 0; i < NUM_BODIES; i++)
    {
        bodies[i].x = rand() % 1000000000; 
        bodies[i].y = rand() % 1000000000;  
        bodies[i].vx = (rand() % 100 - 50) * 1e3; 
        bodies[i].vy = (rand() % 100 - 50) * 1e3; 
        bodies[i].mass = (rand() % 100 + 1) * 1e24; 
    }
    
    clGetPlatformIDs(1, &platform, NULL);
    
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS)
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    queue = clCreateCommandQueue(context, device, 0, &err);
    char *kernel_source = load_kernel_source("CLKernel.cl");
    program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    free(kernel_source);
    
    kernel_forces = clCreateKernel(program, "compute_forces", &err);
    kernel_velocities = clCreateKernel(program, "update_velocities", &err);
    kernel_positions = clCreateKernel(program, "update_positions", &err);
    
    cl_mem buf_bodies = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_BODIES * sizeof(Body), bodies, &err);
    cl_mem buf_fx = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_BODIES * sizeof(double), NULL, &err);
    cl_mem buf_fy = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_BODIES * sizeof(double), NULL, &err);
    
    clSetKernelArg(kernel_forces, 0, sizeof(cl_mem), &buf_bodies);
    clSetKernelArg(kernel_forces, 1, sizeof(cl_mem), &buf_fx);
    clSetKernelArg(kernel_forces, 2, sizeof(cl_mem), &buf_fy);
    int num_bodies = NUM_BODIES;
    clSetKernelArg(kernel_forces, 3, sizeof(int), &num_bodies);
    clSetKernelArg(kernel_forces, 4, sizeof(double), &G);
    
    clSetKernelArg(kernel_velocities, 0, sizeof(cl_mem), &buf_bodies);
    clSetKernelArg(kernel_velocities, 1, sizeof(cl_mem), &buf_fx);
    clSetKernelArg(kernel_velocities, 2, sizeof(cl_mem), &buf_fy);
    clSetKernelArg(kernel_velocities, 3, sizeof(int), &num_bodies);
    double dt = DT;
    clSetKernelArg(kernel_velocities, 4, sizeof(double), &dt);
    
    clSetKernelArg(kernel_positions, 0, sizeof(cl_mem), &buf_bodies);
    clSetKernelArg(kernel_positions, 1, sizeof(int), &num_bodies);
    clSetKernelArg(kernel_positions, 2, sizeof(double), &dt);
    
    size_t global_work_size = NUM_BODIES;
    //printf("Starting N-body simulation with %d bodies using OpenCL...\n", NUM_BODIES);
    double start_time = wtime();
    
    for (int step = 0; step < 1000; step++)
    {
        clEnqueueNDRangeKernel(queue, kernel_forces, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
        clEnqueueNDRangeKernel(queue, kernel_velocities, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
        clEnqueueNDRangeKernel(queue, kernel_positions, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
    }
    
    clFinish(queue);
    
    double end_time = wtime();
    
    clEnqueueReadBuffer(queue, buf_bodies, CL_TRUE, 0, NUM_BODIES * sizeof(Body), bodies, 0, NULL, NULL);
    
    printf("time: %f\n", end_time - start_time);
    
    clReleaseMemObject(buf_bodies);
    clReleaseMemObject(buf_fx);
    clReleaseMemObject(buf_fy);
    clReleaseKernel(kernel_forces);
    clReleaseKernel(kernel_velocities);
    clReleaseKernel(kernel_positions);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    free(bodies);
    free(fx);
    free(fy);
    
    return 0;
}
