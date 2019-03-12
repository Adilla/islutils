#include "tmm_including_transpose_loop_kernel.hu"
__global__ void kernel0(int *B, int alpha)
{
    int b0 = blockIdx.x;
    int t0 = threadIdx.x;

    #define ppcg_min(x,y)    ({ __typeof__(x) _x = (x); __typeof__(y) _y = (y); _x < _y ? _x : _y; })
    #define ppcg_max(x,y)    ({ __typeof__(x) _x = (x); __typeof__(y) _y = (y); _x > _y ? _x : _y; })
    for (int c1 = ppcg_max(0, 32 * b0 - 1024); c1 <= ppcg_min(1023, 32 * b0 + 31); c1 += 32) {
      for (int c3 = ppcg_max(0, 32 * b0 + t0 - c1 - 1023); c3 <= ppcg_min(31, 32 * b0 + t0 - c1); c3 += 1)
        B[(c1 + c3) * 1024 + (32 * b0 + t0 - c1 - c3)] = B[(32 * b0 + t0 - c1 - c3) * 1024 + (c1 + c3)];
      __syncthreads();
    }
}
__global__ void kernel1(int *A, int *B, int *D, int alpha)
{
    int b0 = blockIdx.y, b1 = blockIdx.x;
    int t0 = threadIdx.y, t1 = threadIdx.x;
    __shared__ int shared_A[32][32];
    __shared__ int shared_B[32][32];
    int private_D[1][2];

    {
      private_D[0][0] = D[(32 * b0 + t0) * 1024 + (32 * b1 + t1)];
      private_D[0][1] = D[(32 * b0 + t0) * 1024 + (32 * b1 + t1 + 16)];
      for (int c2 = 0; c2 <= 1023; c2 += 32) {
        for (int c4 = t1; c4 <= 31; c4 += 16)
          shared_A[t0][c4] = A[(32 * b0 + t0) * 1024 + (c2 + c4)];
        for (int c4 = t1; c4 <= 31; c4 += 16)
          shared_B[t0][c4] = B[(32 * b1 + t0) * 1024 + (c2 + c4)];
        __syncthreads();
        for (int c3 = 0; c3 <= 31; c3 += 1) {
          private_D[0][0] += (((alpha) * shared_A[t0][c3]) * shared_B[t1][c3]);
          private_D[0][1] += (((alpha) * shared_A[t0][c3]) * shared_B[t1 + 16][c3]);
        }
        __syncthreads();
      }
      D[(32 * b0 + t0) * 1024 + (32 * b1 + t1)] = private_D[0][0];
      D[(32 * b0 + t0) * 1024 + (32 * b1 + t1 + 16)] = private_D[0][1];
    }
}
