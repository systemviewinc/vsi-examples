#include "complex.h"


void vector_complex_mac_msc(float * __restrict__ A, float * __restrict__ B, float * __restrict__ D, float * __restrict__ C0, float * __restrict__ C1) {
	complex<float> * __restrict Ai = (complex<float> * __restrict__)A;
	complex<float> * __restrict Bi = (complex<float> * __restrict__)B;
        complex<float> * __restrict Di = (complex<float> * __restrict__)D;
	complex<float> * __restrict C0o = (complex<float> * __restrict__)C0;
	complex<float> * __restrict C1o = (complex<float> * __restrict__)C1;

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C0o[i] = Di[i] + Ai[i] * Bi[i];

	#pragma clang loop vectorize(enable) //interleave_count(2)
			for (int i = 0 ; i < 64; i++)
				C1o[i] = Di[i] - Ai[i] * Bi[i];
}
