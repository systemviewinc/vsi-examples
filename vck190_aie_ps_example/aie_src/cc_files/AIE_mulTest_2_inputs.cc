void AIE_mulTest(float * __restrict__ A,float * __restrict__ B, float * __restrict__ C) {
    
    #pragma clang loop vectorize(enable) 
    for (int i = 0 ; i < 8; i++)
        C[i] = A[i]*B[i];
} 
