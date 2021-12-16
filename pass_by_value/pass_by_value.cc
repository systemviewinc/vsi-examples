#include <unistd.h>
#include "pass_by_value.hh"


//#define WRITE_SIZE 40960
#define WRITE_SIZE 1024 * 4


void kernel_gzip(hls::stream<ap_uint<K_WIDTH > > &in_stream,
		 uint32_t in_size,
		 hls::stream<uint32_t> &out_size,
		 uint512_t * outmem) {

	ap_axis_dk<DATA_WIDTH> out;
	ap_axis_dk<DATA_WIDTH> in;

	int end = 0;
	//if we recieve data then we will "process" it
	//  while there is data read it out, once we are empty generate our data
	static uint512_t output_array[128];
	printf("sizeof output array %d, %d\n",sizeof(output_array),in_size);

	//read insize
	uint32_t i_size = in_size;

	//number of reads = i_size / K_WIDTH rounded up
	int in_read = (i_size + K_WIDTH - 1 )/K_WIDTH;


	while(in_read-- > 0){
#pragma HLS PIPELINE II=1
		volatile ap_uint<K_WIDTH> temp = in_stream.read();
		output_array[in_read%128] = (uint512_t)temp;
	}

	memcpy(outmem, output_array, sizeof(output_array));

	//write 16 times so the data doesn't get stuck in the upsizer
	for(int i = 0; i < 16; i++) {
#pragma HLS PIPELINE II=1
		out_size.write(in_read);
	}
}

#ifndef __VSI_HLS_SYN__

void trigger_kernel(hls::stream<ap_uint<K_WIDTH > > &out_stream,
		    vsi::device<int>  &out_size,
		    hls::stream<uint32_t> &in_size,
		    vsi::device<int> &inmem) {
	int out_write = (WRITE_SIZE + K_BYTES - 1 )/K_BYTES;
	printf("Write size is 0x%08x \n", WRITE_SIZE);
	printf("#of writes is 0x%08x %d\n", WRITE_SIZE,out_write * sizeof(ap_uint<K_WIDTH >));
	uint512_t * write_array = malloc(out_write * sizeof(ap_uint<K_WIDTH >));



	//send the size we want to write
	printf("Write size stream %d\n",WRITE_SIZE);
	uint32_t o_size = WRITE_SIZE;
	out_size.pwrite(&o_size,sizeof(o_size),0);


	sleep(5);

	printf("Write data stream \n");
	out_stream.write(write_array,WRITE_SIZE);

	printf("Read size stream \n");

	uint32_t return_size = in_size.read();

	printf("Size: 0x%08x\n",return_size);

	exit(0);


}
#endif
