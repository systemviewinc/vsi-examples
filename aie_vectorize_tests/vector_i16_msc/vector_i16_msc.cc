#include <stdint.h>

void vector_i16_msc(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ C) {
int16_t val10 = 10;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = val10 - A[i] * B[i];
	} 		 
}
