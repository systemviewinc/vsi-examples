#include "complex.h"
using namespace std;

void vector_complex_mult(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C) {
	complex<int32_t> * __restrict Ai = (complex<int32_t> * __restrict__)A; 
	complex<int32_t> * __restrict Co = (complex<int32_t> * __restrict__)C;
	
#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		Co[i] = Ai[i] * B[i];
}
