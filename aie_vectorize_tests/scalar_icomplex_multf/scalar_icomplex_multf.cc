#include "complex.h"


void scalar_icomplex_multf(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C) {
	complex<int32_t> * __restrict Ai = (complex<int32_t> * __restrict__)A;
	complex<int32_t> * __restrict Co = (complex<int32_t> * __restrict__)C;

#pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 64; i++)
		Co[i] = Ai[i] * B[i];
}
