#include <stdint.h>
using namespace std;
void double_int(int32_t * __restrict__ A, int32_t * __restrict__ C) {
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
          C[i] = A[i] + A[i];  // int32 double
	} 		 
}
