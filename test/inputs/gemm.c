int D[1024][1024];
int C[1024][1024];
int A[1024][1024];
int B[1024][1024];
int alpha, beta;

void gemm() {

  for (int i = 0; i < 1024; i++)
    for (int j = 0; j < 1024; j++) 
        D[i][j] = beta * C[i][j];

#pragma scop
  for (int i = 0; i < 1024; i++)
    for (int j = 0; j < 1024; j++) 
        for (int k = 0; k < 1024; k++) 
            D[i][j] += alpha * A[i][k] * B[k][j];        
#pragma endscop
}
