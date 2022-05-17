#include <stdint.h>
void gui_test(int32_t * __restrict__ A , int32_t * __restrict__ B , int32_t * __restrict__ C) {
	  #pragma clang loop vectorize(enable)
   	for (int i = 0; i < 8; ++i) {
       		    #pragma clang loop vectorize(enable)
          		for (int k = 0; k < 16; ++k)
            				C[i*16 + k] = A[i*16 + k] + B[i*16 + k];
    }
}
