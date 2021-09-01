#include "_complex.h"


void unalignLoad_complexFloat(_complex<float> * __restrict__ A, _complex<float> * __restrict__ B, _complex<float> * __restrict__ C) {

	 int countB=0;
	_complex<float> tmp_1 = B[countB++];

	#pragma clang loop vectorize(enable) interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		  C[i] = tmp_1 + A[i] * B[countB++]; 
	}
}
