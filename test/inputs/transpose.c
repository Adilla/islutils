int B[1024][1024];

void transpose() {

#pragma scop
for (int i = 0; i < 1024; i++)
    for (int j = 0; j < 1024; j++) 
        B[i][j] = B[j][i];
#pragma endscop

}
