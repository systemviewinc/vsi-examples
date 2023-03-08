#include <stdint.h>
using namespace std;
void rtp_scalarInt16(const int16_t &A, int16_t * __restrict__ B0,  int16_t * __restrict__ B1, int16_t  * __restrict__ C,  int16_t  * __restrict__ D) {

#pragma clang loop vectorize(enable) interleave_count(1)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A;
	 }

#pragma clang loop vectorize(disable) interleave_count(1)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A;
	 }
}
