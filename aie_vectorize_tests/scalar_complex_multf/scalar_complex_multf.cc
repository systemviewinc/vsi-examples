#include "_complex.h"


void scalar_complex_multf(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
	_complex<float> * __restrict Ai = (_complex<float> * __restrict__)A;
	_complex<float> * __restrict Co = (_complex<float> * __restrict__)C;

#pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 64; i++)
		Co[i] = Ai[i] * B[i];
}
