  #include <stdint.h>

	#define NUM_DATA_IN_EACH_CHANNEL 63

	void memcpy_window_scalarLoad(
	    float * __restrict__ win1_in,
	    float * __restrict__ win2_in,
	    float * __restrict__ win3_in,
	    float * __restrict__ win4_in,
		float * __restrict__ win_out) {

	        for (int i = 0; i < NUM_DATA_IN_EACH_CHANNEL; i++ ) {
	            win_out[i] = win1_in[i];
	        }
	        for (int i = 0; i < NUM_DATA_IN_EACH_CHANNEL; i++ ) {
	            win_out[NUM_DATA_IN_EACH_CHANNEL+ i] = win2_in[i];
	        }
	        for (int i = 0; i < NUM_DATA_IN_EACH_CHANNEL; i++ ) {
	            win_out[2*NUM_DATA_IN_EACH_CHANNEL+ i] = win3_in[i];
	        }
	        for (int i = 0; i < NUM_DATA_IN_EACH_CHANNEL; i++ ) {
	            win_out[3*NUM_DATA_IN_EACH_CHANNEL+ i] = win4_in[i];
	        }
	}
