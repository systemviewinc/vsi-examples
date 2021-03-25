#include <stdint.h>
#include "math.h"

void scalar_sin_cos(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {

#pragma clang loop vectorize(disable) 
	for (int i = 0 ; i < 64; i++) {
        C[i] = sinf(A[i]) + cosf(B[i]);
	}		 
}
