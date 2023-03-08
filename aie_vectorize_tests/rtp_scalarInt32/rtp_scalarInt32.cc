#include <stdint.h>
using namespace std;
void rtp_scalarInt32(const int32_t &A, int32_t * __restrict__ B0,  int32_t * __restrict__ B1, int32_t  * __restrict__ C,  int32_t  * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A;
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A;
	 }
}
