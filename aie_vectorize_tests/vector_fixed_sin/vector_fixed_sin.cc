#include <stdint.h>
#include <ap_int.h>

void vector_fixed_sin(ap_int<1,15> * __restrict__ A,
					ap_int<1,15> * __restrict__ C) {
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
          C[i] = A[i].sin();  // fixed sin
	} 		 
}
