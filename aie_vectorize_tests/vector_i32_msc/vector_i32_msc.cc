#include <stdint.h>

void vector_i32_msc(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C) {
int32_t val10 = 10;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = val10 - A[i] * B[i];
	} 		 
}
