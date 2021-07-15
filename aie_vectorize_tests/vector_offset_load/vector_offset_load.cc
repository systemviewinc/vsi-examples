#include <stdint.h>

void vector_offset_load(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
	int countA=0;
	int countB=0;
	float tmp_0 = A[countA++] + A[countA++] + A[countA++] + A[countA++];
	float tmp_1 = B[countB++] + B[countB++] + B[countB++] + B[countB++] + B[countB++];

#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		  C[i] = tmp_0 * tmp_1 + A[countA++] * B[countB++];  // float mul
	}
}
