void AIE_mulTest(float * __restrict__ A, float * __restrict__ C) {
    float B=6.28318530718f;
    #pragma clang loop vectorize(enable) 
    for (int i = 0 ; i < 8; i++)
        C[i] = A[i]*B;
} 
