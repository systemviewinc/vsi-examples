#include <stdint.h>
using namespace std;
void rtp_arrayFloat(const float (&A)[64], float* __restrict__ B0,  float* __restrict__ B1, float * __restrict__ C,  float * __restrict__ D) {

#pragma clang loop vectorize(enable)
 	for (int i = 0 ; i < 64; i++) {
           C[i] = B0[i] * A[i];
	 }

#pragma clang loop vectorize(disable)
 	for (int i = 0 ; i < 64; i++) {
           D[i] = B1[i] * A[i];
	 }
}
