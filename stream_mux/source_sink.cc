#include <stream_mux.h>
#include <hls_stream_types.h>
#include <stdio.h>
#include <unistd.h>
void source_sink(hls::stream<unsigned int> &in, hls::stream<unsigned int> &out) {
	// sendo out 4 k bytes then loop everything coming in
	for (int i = 0 ; i < 1024; i ++ ) out.write(i);
	while (1) {
		out.write(in.read());
	}
}

void source(hls::stream<unsigned int> &out) {
	// sendo out 4 k bytes then exit
	for (int i = 0 ; i < 1024; i ++ ) 
#pragma HLS pipeline II=1
		out.write(i);
}

unsigned int buff[1024];

void sink(hls::stream<unsigned int>&in) {
	while (1) {
		for (int i = 0 ; i < 1024; i ++ ) 
#pragma HLS pipeline II=1
			buff[i] = in.read();
		printf("%s 4k received\n",__FUNCTION__);
	}
}

/**
 * @brief just a pass through
 *
 * @param in_arr
 * @param out_arr
 */
void pass_thru_ss(int in_arr[1024], int out_arr[1024])
{
	printf("%s started\n", __FUNCTION__);
	for (int i =0 ; i < 1024; i++) {
#pragma HLS PIPELINE II=1
		out_arr[i] = in_arr[i];
	}
	printf("%s done\n", __FUNCTION__);
}

/**
 * @brief just a pass through : reads 1024 ints form the source
 *        and write to output
 *
 * @param in_arr
 * @param out_arr
 */
void pass_thru_fixed_stream(hls::stream<ap_axis_d<32>> &in, hls::stream<ap_axis_d<32>> &out) {
	for (int i = 0 ; i < 1024 ; i++) {
#pragma HLS PIPELINE II=1
		ap_axis_d<32> d = in.read();
		d.last = (i==1023);
		out.write(d);
	}
}


/**
 * @brief Generate packages sawtooth wave data.
 *
 * @param out - output stream.
 */
void generator(hls::stream<ap_axis_d <32>> &out) {
#define PACKAGE_SIZE 1024
	ap_axis_d <32> tmp_out;
	for (int i = 0; i < PACKAGE_SIZE; i++ ) {
		tmp_out.data = i;
		tmp_out.last = (i == PACKAGE_SIZE - 1);
#pragma HLS pipeline II=1
		out.write(tmp_out);
	}
	usleep(10000);
	printf("%s: sent data\n",__FUNCTION__);
}

<<<<<<< HEAD
=======
/**
 * @brief Generate data for AIe Viterbi decoder.
 * 
 * @param out - output stream.
 */
void generator_viterbi_decoder(hls::stream<ap_axis_d <32>> &out) {
  #define PACKAGE_SIZE 1024
  int data[] ={28, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1};
  static int data_poi = 0;
  ap_axis_d <32> tmp_out;
  for (int i = 0; i < PACKAGE_SIZE; i++ ) {
    tmp_out.data = data[data_poi];
    if (data_poi == 28) {
      data_poi = 0;
    } else {
      data_poi++;
    }
    tmp_out.last = (i == PACKAGE_SIZE - 1);
#pragma HLS pipeline II=1
    out.write(tmp_out);
  }
}

/**
 * @brief Print out the data coming from AIe Viterbi decoder.
 * 
 * @param in - input stream.
 */
void sink_viterbi_decoder(hls::stream<unsigned int>&in) {
  while (1) {
    for (int i = 0 ; i < 1024; i ++ ) {
#pragma HLS pipeline II=1
      buff[i] = in.read();
      printf("%d \n", buff[i]);
    }
  }
}
>>>>>>> 3c657f8bad3a536657c57ce993aa597149cf78cf
