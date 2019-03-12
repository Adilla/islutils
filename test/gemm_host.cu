#include <assert.h>
#include <stdio.h>
#include "gemm_kernel.hu"
int D[1024][1024];
int C[1024][1024];
int A[1024][1024];
int B[1024][1024];
int alpha, beta;

void gemm() {

  for (int i = 0; i < 1024; i++)
    for (int j = 0; j < 1024; j++) 
        D[i][j] = beta * C[i][j];

  {
#define cudaCheckReturn(ret) \
  do { \
    cudaError_t cudaCheckReturn_e = (ret); \
    if (cudaCheckReturn_e != cudaSuccess) { \
      fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString(cudaCheckReturn_e)); \
      fflush(stderr); \
    } \
    assert(cudaCheckReturn_e == cudaSuccess); \
  } while(0)
#define cudaCheckKernel() \
  do { \
    cudaCheckReturn(cudaGetLastError()); \
  } while(0)

    cublasHandle_t handle;
    int *dev_A;
    int *dev_B;
    int *dev_D;
    
    cudaCheckReturn(cublasCreate(&handle));
    cudaCheckReturn(cudaMalloc((void **) &dev_A, (1024) * (1024) * sizeof(int)));
    cudaCheckReturn(cudaMalloc((void **) &dev_B, (1024) * (1024) * sizeof(int)));
    cudaCheckReturn(cudaMalloc((void **) &dev_D, (1024) * (1024) * sizeof(int)));
    
    cudaCheckReturn(cublasSetMatrix(1024, 1024, sizeof(int), A, 1024, dev_A, 1024));
    cudaCheckReturn(cublasSetMatrix(1024, 1024, sizeof(int), B, 1024, dev_B, 1024));
    cudaCheckReturn(cublasSetMatrix(1024, 1024, sizeof(int), D, 1024, dev_D, 1024));
    {
      dim3 k0_dimBlock(16, 32);
      dim3 k0_dimGrid(32, 32);
      kernel0 <<<k0_dimGrid, k0_dimBlock>>> (dev_A, dev_B, dev_D, alpha);
      cudaCheckKernel();
    }
    
    cudaCheckReturn(cudaGetMatrix(1024, 1024, sizeof(int), dev_D, 1024, D, 1024));
    cudaCheckReturn(cudaFree(dev_A));
    cudaCheckReturn(cudaFree(dev_B));
    cudaCheckReturn(cudaFree(dev_D));
    cudaCheckReturn(cublasDestroy(handle));
  }
}
