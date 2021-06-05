#include "_complex.h"


void tf_cfloat_window(_complex<float> * __restrict__ A, _complex<float> * __restrict__ B, _complex<float> * __restrict__ C0, _complex<float> * __restrict__ C1) {


        #pragma clang loop vectorize(disable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C0[i] =  A[i] + B[i];

        #pragma clang loop vectorize(enable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C1[i] =  A[i] + B[i];
}
