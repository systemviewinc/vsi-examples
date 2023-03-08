#include <stdint.h>
using namespace std;
void vector_i32_mac(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C) {
int32_t val10 = 10;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = A[i] + B[i] * val10;
	} 		 
}
