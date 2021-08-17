#include <stdint.h>

void rtp_arrayInt32(const int32_t (&A)[64], int32_t * __restrict__ B0,  int32_t * __restrict__ B1, int32_t * __restrict__ C,  int32_t * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A[i];
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A[i];
	 }
}
