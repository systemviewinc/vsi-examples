#include <stdint.h>
#include "math.h"

void vector_scalar_sqrt(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {

	float tmp = sqrtf(B[9]);

    #pragma clang loop vectorize(enable) 
	for (int i = 0 ; i < 64; i++) {
        C[i] = sqrtf(A[i]) + tmp;
	}		 
}

