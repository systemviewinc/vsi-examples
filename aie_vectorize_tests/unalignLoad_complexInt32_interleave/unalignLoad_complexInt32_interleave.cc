#include "_complex.h"


void unalignLoad_complexInt32_interleave(_complex<int32_t> * __restrict__ A, _complex<int32_t> * __restrict__ B, _complex<int32_t> * __restrict__ C) {

	 int countB=0;
	_complex<int32_t> tmp_1 = B[countB++];

	#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++) {
		  C[i] = tmp_1 + B[countB++] * A[i];
	}
}
