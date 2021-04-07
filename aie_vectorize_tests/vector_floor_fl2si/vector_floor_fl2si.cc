#include <stdint.h>
#include "math.h"

void vector_floor_fl2si(float * __restrict__ A, int32_t * __restrict__ C) {

	#pragma clang loop vectorize(enable) 
	for (int i = 0 ; i < 64; i++) {
		C[i] = floorf(A[i]);
	}	 
}
