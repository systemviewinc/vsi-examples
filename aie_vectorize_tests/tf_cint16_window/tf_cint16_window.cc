#include "complex.h"


void tf_cint16_window(complex<int16_t> * __restrict__ A, complex<int16_t> * __restrict__ B, complex<int16_t> * __restrict__ C0, complex<int16_t> * __restrict__ C1) {


        #pragma clang loop vectorize(disable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C0[i] =  A[i] + B[i];

        #pragma clang loop vectorize(enable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C1[i] =  A[i] + B[i];
}
