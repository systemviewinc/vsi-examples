#include <stdint.h>
#include "math.h"

void vector_scalar_fabs(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {

	float tmp = fabs(B[9]);

    #pragma clang loop vectorize(enable) 
	for (int i = 0 ; i < 64; i++) {
        C[i] = fabs(A[i]) + tmp;
	}		 
}

