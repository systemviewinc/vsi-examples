#include <stdint.h>
using namespace std;
void rtp_arrayInt16(const int16_t (&A)[64], int16_t * __restrict__ B0,  int16_t * __restrict__ B1, int16_t * __restrict__ C,  int16_t * __restrict__ D) {

#pragma clang loop vectorize(enable) interleave_count(1)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A[i];
	 }

#pragma clang loop vectorize(disable) interleave_count(1)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A[i];
	 }
}
