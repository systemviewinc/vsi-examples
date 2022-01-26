#include "_complex.h"


void tf_cint32_stream(_complex<int32_t> * __restrict__ A, _complex<int32_t> * __restrict__ B, _complex<int32_t> * __restrict__ C0, _complex<int32_t> * __restrict__ C1) {


        #pragma clang loop vectorize(disable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C0[i] =  A[i] + B[i];

        #pragma clang loop vectorize(enable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C1[i] =  A[i] + B[i];
}