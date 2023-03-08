#include "complex.h"
using namespace std;

void vector_i16complex_add_sub(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ C0, int16_t * __restrict__ C1) {
	complex<int16_t> * __restrict Ai = (complex<int16_t> * __restrict__)A;
	complex<int16_t> * __restrict Bi = (complex<int16_t> * __restrict__)B;
	complex<int16_t> * __restrict C0o = (complex<int16_t> * __restrict__)C0;
	complex<int16_t> * __restrict C1o = (complex<int16_t> * __restrict__)C1;

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 32; i++)
		C0o[i] = Ai[i] + Bi[i];

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 32; i++)
		C1o[i] = Ai[i] - Bi[i];

}
