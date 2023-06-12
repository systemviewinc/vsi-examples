#include <stdint.h>
#include <ap_int.h>

void mixed_fixed_point( ap_int<1,15> * __restrict__ A1, 
						ap_int<1,15> * __restrict__ B1, 
						ap_int<1,15> * __restrict__ C1,
						ap_int<1,15> * __restrict__ D1,
						ap_int<8,8> * __restrict__ A2, 
						ap_int<8,8> * __restrict__ B2, 
						ap_int<8,8> * __restrict__ C2,
                    	ap_int<8,8> * __restrict__ D2) {

	#pragma clang loop vectorize(enable)
    for (int i = 0 ; i < 64; i++) {

        D1[i] = A1[i] + B1[i] * C1[i];  // fixed mac ap_int<1,15>
        D2[i] = A2[i] + B2[i] * C2[i];  // fixed mac ap_int<8,8>

	}
}