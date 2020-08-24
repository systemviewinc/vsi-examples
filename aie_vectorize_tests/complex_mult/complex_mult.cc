#include "_complex.h"


void complex_mult(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
	_complex * __restrict Ai = (_complex * __restrict__)A; 
	_complex * __restrict Bi = (_complex * __restrict__)B; 
	_complex * __restrict Co = (_complex * __restrict__)C;
	
#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		Co[i] = Ai[i]*Bi[i];
}
