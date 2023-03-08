#include <stdint.h>
using namespace std;

void localMemAlign(float * __restrict__ A, float * __restrict__ B,  float * __restrict__ C0, float * __restrict__ C1) {

       float localMem [64];

       for (int i = 0 ; i < 64; i++)
          localMem[i] = A[i];

    	#pragma clang loop vectorize(disable)
			for (int i = 0 ; i < 64; i++)
    			C0[i] =   localMem[i] + B[i];

    	#pragma clang loop vectorize(enable)
			for (int i = 0 ; i < 64; i++)
    			C1[i] =   localMem[i] + B[i];

}
