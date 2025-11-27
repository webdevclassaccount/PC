#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define N 1000

void displayMatrix(int** matrix, int n);
void matrixMultiply(int** A, int** B, int** C, int n);
