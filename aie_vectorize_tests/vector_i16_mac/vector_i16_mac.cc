#include <stdint.h>
using namespace std;
void vector_i16_mac(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ C) {
int16_t val10 = 10;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = A[i] + B[i] * val10;
	} 		 
}
