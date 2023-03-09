#include "complex.h"
using namespace std;

void vector_cfloat_multf(cfloat * __restrict__ A, float* __restrict__ B, cfloat * __restrict__ C) {

#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C[i] = A[i] * B[i];
}
