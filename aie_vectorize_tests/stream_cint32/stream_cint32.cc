#include "complex.h"


void stream_cint32(int32_t * __restrict__ A, int32_t * __restrict__ B,  int32_t * __restrict__ C0, int32_t * __restrict__ C1) {
        complex<int32_t> * __restrict Ai = (complex<int32_t> * __restrict__)A;
        complex<int32_t> * __restrict Bi = (complex<int32_t> * __restrict__)B;
        complex<int32_t> * __restrict C0o = (complex<int32_t> * __restrict__)C0;
        complex<int32_t> * __restrict C1o = (complex<int32_t> * __restrict__)C1;


        #pragma clang loop vectorize(disable)
        for (int i = 0 ; i < 64; i++)
            C0o[i] =   Ai[i] + Bi[i];

        #pragma clang loop vectorize(enable)
        for (int i = 0 ; i < 64; i++)
            C1o[i] =   Ai[i] + Bi[i];
}
