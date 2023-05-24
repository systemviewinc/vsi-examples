#include <stdint.h>
#include <ap_int.h>

void vector_fixed_msc(int * __restrict__ A, int * __restrict__ B, int * __restrict__ C) {
ap_int<1,15> * Af = (ap_int<1,15>*)A;
ap_int<1,15> * Bf = (ap_int<1,15>*)B;
ap_int<1,15> * Cf = (ap_int<1,15>*)C;
ap_int<1,15> val(16384);
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		  Cf[i] = Af[i] - Bf[i] * val;  // fixed msc
	} 		 
}
