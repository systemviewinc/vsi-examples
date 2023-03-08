#include "complex.h"
using namespace std;
void rtp_scalar_cfloat(const complex<float> &A, complex<float> * __restrict__ B0,  complex<float> * __restrict__ B1, complex<float> * __restrict__ C,  complex<float> * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A;
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A;
	 }
}
