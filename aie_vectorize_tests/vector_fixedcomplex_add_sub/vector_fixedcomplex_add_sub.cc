#include "complex.h"
using namespace std;

void vector_fixedcomplex_add_sub(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C0, int32_t * __restrict__ C1) {
	complex<ap_int<1,15>> * __restrict Ai = (complex<ap_int<1,15>> * __restrict__)A;
	complex<ap_int<1,15>> * __restrict Bi = (complex<ap_int<1,15>> * __restrict__)B;
	complex<ap_int<1,15>> * __restrict C0o = (complex<ap_int<1,15>> * __restrict__)C0;
	complex<ap_int<1,15>> * __restrict C1o = (complex<ap_int<1,15>> * __restrict__)C1;

	// complex<ap_int<1,15>> tmp = Ai[0] + Ai[1];
#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C0o[i] = Ai[i] + Bi[i];

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C1o[i] = Ai[i] - Bi[i];

}
