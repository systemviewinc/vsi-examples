#include "complex.h"


void unalignLoad_complexFloat(complex<float> * __restrict__ A, complex<float> * __restrict__ B, complex<float> * __restrict__ C) {

	 int countB=0;
	complex<float> tmp_1 = B[countB++];

	#pragma clang loop vectorize(enable) interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		  C[i] = tmp_1 + A[i] * B[countB++]; 
	}
}
