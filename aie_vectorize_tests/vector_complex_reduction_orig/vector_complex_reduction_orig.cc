#include "_complex.h"

void complexReduction(float * __restrict__ A, float * __restrict__ C) {

	_complex<float> * __restrict Ai = (_complex<float> * __restrict__)A; 
	_complex<float> * __restrict Co = (_complex<float> * __restrict__)C;

    _complex<float> accum; 
    accum.real() = 0.0f;
    accum.imag() = 0.0f;
    #pragma clang loop vectorize(enable)            
    for (int i = 0; i < 8; ++i)
    {
        accum.real() += Ai[i].real();
        accum.imag() += Ai[i].imag();
    }

    Co[0] = accum;
}