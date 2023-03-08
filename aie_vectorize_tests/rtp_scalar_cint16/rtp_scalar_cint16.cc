#include "complex.h"
using namespace std;
void rtp_scalar_cint16(const complex<int16_t> &A, complex<int16_t> * __restrict__ B0,  complex<int16_t> * __restrict__ B1, complex<int16_t> * __restrict__ C,  complex<int16_t> * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A;
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A;
	 }
}
