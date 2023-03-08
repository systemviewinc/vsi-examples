#include "complex.h"
using namespace std;

void scalar_complex_add(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
	complex<float> * __restrict Ai = (complex<float> * __restrict__)A;
	complex<float> * __restrict Bi = (complex<float> * __restrict__)B;
	complex<float> * __restrict Co = (complex<float> * __restrict__)C;

#pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 64; i++)
		Co[i] = Ai[i] + Bi[i];
}
