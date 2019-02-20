int A[1024];
int B[1024];

void kernel_jacobi_1d() {

#pragma scop
  for (int t = 0; t < 2; t++) {
    for (int i = 1; i < 1024; i++)
	    B[i] = 0.33333 * (A[i-1] + A[i] + A[i + 1]);
    for (int i = 1; i < 1024; i++)
	    A[i] = 0.33333 * (B[i-1] + B[i] + B[i + 1]);
    }
#pragma endscop

}
