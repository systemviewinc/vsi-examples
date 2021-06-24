#include "_complex.h"


void vector_i16complex_mac_msc(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ D, int16_t * __restrict__ C0, int16_t * __restrict__ C1) {
	_complex<int16_t> * __restrict Ai = (_complex<int16_t> * __restrict__)A;
	_complex<int16_t> * __restrict Bi = (_complex<int16_t> * __restrict__)B;
        _complex<int16_t> * __restrict Di = (_complex<int16_t> * __restrict__)D;
	_complex<int16_t> * __restrict C0o = (_complex<int16_t> * __restrict__)C0;
	_complex<int16_t> * __restrict C1o = (_complex<int16_t> * __restrict__)C1;

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C0o[i] = Di[i] + Ai[i] * Bi[i];

	#pragma clang loop vectorize(enable) //interleave_count(2)
			for (int i = 0 ; i < 64; i++)
				C1o[i] = Di[i] - Ai[i] * Bi[i];
}
