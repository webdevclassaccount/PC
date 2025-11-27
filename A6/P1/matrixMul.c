#include "matrixMul.h"
#include <string.h>

const char* kernelSource = 
"__kernel void matrixMultiplyKernel(__global const int* A,\n"
"                                    __global const int* B,\n"
"                                    __global int* C,\n"
"                                    const int n)\n"
"{\n"
"    int row = get_global_id(0);\n"
"    int col = get_global_id(1);\n"
"    \n"
"    if (row < n && col < n) {\n"
"        int sum = 0;\n"
"        for (int k = 0; k < n; k++) {\n"
"            sum += A[row * n + k] * B[k * n + col];\n"
"        }\n"
"        C[row * n + col] = sum;\n"
"    }\n"
"}\n";

void displayMatrix(int** matrix, int n) 
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
            printf("%d ", matrix[i][j]);

        printf("\n");
    }
}

void matrixMultiply(int** A, int** B, int** C, int n) 
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem bufferA, bufferB, bufferC;
    
    int* flatA = (int*)malloc(n * n * sizeof(int));
    int* flatB = (int*)malloc(n * n * sizeof(int));
    int* flatC = (int*)malloc(n * n * sizeof(int));
    
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            flatA[i * n + j] = A[i][j];
            flatB[i * n + j] = B[i][j];
            flatC[i * n + j] = 0;
        }
    }
    
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS)
    {
        //printf("Failed to get platform\n");
        return;
    }
    
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS)
    {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        if (err != CL_SUCCESS)
        {
            //printf("Failed to get device\n");
            return;
        }
    }
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS)
    {
        //printf("Failed to create context\n");
        return;
    }
    
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS)
    {
        //printf("Failed to create command queue\n");
        clReleaseContext(context);
        return;
    }
    
    size_t sourceSize = strlen(kernelSource);
    program = clCreateProgramWithSource(context, 1, &kernelSource, &sourceSize, &err);
    if (err != CL_SUCCESS)
    {
        //printf("Failed to create program\n");
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }
    
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* log = (char*)malloc(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        //printf("Build error:\n%s\n", log);
        free(log);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }
    
    kernel = clCreateKernel(program, "matrixMultiplyKernel", &err);
    if (err != CL_SUCCESS)
    {
        //printf("Failed to create kernel\n");
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }
    
    bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, n * n * sizeof(int), flatA, &err);
    bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, n * n * sizeof(int), flatB, &err);
    bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, n * n * sizeof(int), NULL, &err);
    
    if (err != CL_SUCCESS)
    {
        //printf("Failed to create buffers\n");
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }
    
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    clSetKernelArg(kernel, 3, sizeof(int), &n);
    
    size_t globalSize[2] = {n, n};
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        //printf("Failed to enqueue kernel: %d\n", err);
    }
    else
    {
        clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, n * n * sizeof(int), flatC, 0, NULL, NULL);
        
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
                C[i][j] = flatC[i * n + j];
        }
    }
    
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    free(flatA);
    free(flatB);
    free(flatC);
}

int main() 
{
    int** A = (int**)malloc(N * sizeof(int*));
    int** B = (int**)malloc(N * sizeof(int*));
    int** C = (int**)malloc(N * sizeof(int*));
    
    for (int i = 0; i < N; ++i)
    {
        A[i] = (int*)malloc(N * sizeof(int));
        B[i] = (int*)malloc(N * sizeof(int));
        C[i] = (int*)malloc(N * sizeof(int));
    }

    if (A == NULL || B == NULL || C == NULL)
        return -1;

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 0;
        }
    }

    matrixMultiply(A, B, C, N);

    printf("Matrix multiplication complete!\n");

    for (int i = 0; i < N; ++i)
    {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    return 0;
}
