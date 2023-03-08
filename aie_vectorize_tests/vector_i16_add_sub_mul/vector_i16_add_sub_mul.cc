#include <stdint.h>
using namespace std;
void vector_i16_add_sub_mul(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ C) {
int16_t tmp_0;
int16_t tmp_1;
int16_t val10 = 2;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
          tmp_0 = A[i] + val10;  // int16 add
		  tmp_1 = B[i] - val10;  // int16 sub
		  C[i] = tmp_0 * tmp_1;  // int16 mul
	} 		 
}
