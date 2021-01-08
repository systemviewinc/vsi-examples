#include "_complex.h"

void complexReduction(float * __restrict__ A, float * __restrict__ C) {

	_complex<float> * __restrict Ai = (_complex<float> * __restrict__)A; 
	_complex<float> * __restrict Co = (_complex<float> * __restrict__)C;

    _complex<float> accum(0.0f,0.0f);
    #pragma clang loop vectorize(enable)            
    for (int i = 0; i < 8; ++i)
    {
        accum += Ai[i];
    }

    Co[0] = accum;
}