#include <stdint.h>

void vector_float_msc(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
float val10 = 10.0f;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = val10 - A[i] * B[i];
	} 		 
}
