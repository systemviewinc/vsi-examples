#include "complex.h"
using namespace std;

void unalignLoad_complexInt32(complex<int32_t> * __restrict__ A, complex<int32_t> * __restrict__ B, complex<int32_t> * __restrict__ C) {

	 int countB=0;
	complex<int32_t> tmp_1 = B[countB++];

	#pragma clang loop vectorize(enable) interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		  C[i] = tmp_1 + B[countB++] * A[i];
	}
}
