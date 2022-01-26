#include "_complex.h"

void rtp_scalar_cint32(const _complex<int32_t> &A, _complex<int32_t> * __restrict__ B0,  _complex<int32_t> * __restrict__ B1, _complex<int32_t> * __restrict__ C,  _complex<int32_t> * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A;
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A;
	 }
}