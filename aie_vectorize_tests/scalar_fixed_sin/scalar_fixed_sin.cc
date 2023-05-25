#include <stdint.h>
#include <ap_int.h>

void scalar_fixed_sin(int * __restrict__ A, int * __restrict__ C) {

ap_int<1,15> * Af = (ap_int<1,15>*)A;
ap_int<1,15> * Cf = (ap_int<1,15>*)C;
#pragma clang loop vectorize(disable) //interleave_count(1)
	for (int i = 0 ; i < 1024; i++) {
          Cf[i] = Af[i].sin();  // fixed sin
	} 		 
}
