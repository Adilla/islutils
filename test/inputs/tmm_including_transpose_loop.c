int D[1024][1024];
int C[1024][1024];
int A[1024][1024];
int B[1024][1024];
int alpha, beta;

void tgemm() {

#pragma scop
for (int ii = 0; ii < 1024; ii++)
    for (int jj = 0; jj < 1024; jj++) 
        B[ii][jj] = B[jj][ii];


for (int i = 0; i < 1024; i++)
    for (int j = 0; j < 1024; j++) {
        for (int k = 0; k < 1024; k++) 
            D[i][j] += alpha * A[i][k] * B[j][k];        
    }
#pragma endscop
}
