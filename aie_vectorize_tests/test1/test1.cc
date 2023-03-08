#include <stdint.h>
using namespace std;
int32_t K = 50;
void test_func (int32_t * __restrict__ A __attribute__((aligned(16))),
	   int32_t * __restrict__ B __attribute__((aligned(16))),
	   int32_t * __restrict__ C __attribute__((aligned(16)))) {
#pragma clang loop vectorize(enable)
	for (int i = 0; i < 1024; ++i) {
		C[i] = A[i] * (B[i] + K);
	}
}

