#include "_complex.h"


void scalar_icomplex_cconj(int32_t * __restrict__ A, int32_t * __restrict__ C) {

	_complex<int32_t> * __restrict Ai = (_complex<int32_t> * __restrict__)A;
        _complex<int32_t> * __restrict Co = (_complex<int32_t> * __restrict__)C;

        #pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 64; i++)
		Co[i] = cconj(Ai[i]);
}
