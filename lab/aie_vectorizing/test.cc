static float K=3.1415f;
void top (float A[1024] , 
	  float B[1024] , 
	  float C[1024]) { 
//void top (float * __restrict__ A , 
//	  float * __restrict__ B , 
//	  float * __restrict__ C ) { 
//#pragma clang loop vectorize(enable) 
for (int i = 0; i < 1024; ++i)
    C[i] = (A[i] * B[i]) + K;
    }