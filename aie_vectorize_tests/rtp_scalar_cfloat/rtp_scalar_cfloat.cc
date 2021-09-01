#include "_complex.h"

void rtp_scalar_cfloat(const _complex<float> &A, _complex<float> * __restrict__ B0,  _complex<float> * __restrict__ B1, _complex<float> * __restrict__ C,  _complex<float> * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A;
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A;
	 }
}
