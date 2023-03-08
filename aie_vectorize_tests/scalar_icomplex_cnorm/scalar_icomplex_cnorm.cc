#include "complex.h"
using namespace std;

void scalar_icomplex_cnorm(int32_t * __restrict__ A, int32_t * __restrict__ C) {

	complex<int32_t> * __restrict Ai = (complex<int32_t> * __restrict__)A;

        #pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 64; i++)
		C[i] = cnorm(Ai[i]);
}
