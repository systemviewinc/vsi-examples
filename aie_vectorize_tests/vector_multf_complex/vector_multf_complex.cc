#include "complex.h"
using namespace std;

void vector_multf_complex(complex<float> * __restrict__ A, float* __restrict__ B, complex<float> * __restrict__ C) {

#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C[i] = A[i] * B[i];
}
