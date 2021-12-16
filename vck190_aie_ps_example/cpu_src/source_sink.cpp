
#include <thread>
#include <chrono>
#include <vector>

#include "ai_send_driver.h"

#define NUM_INTS 1024
#define NUM_FLOATS 16

// read is done
bool rd_done = false;

void ints_to_streams (vsi::device<int> &mem_out) {

	int data[NUM_INTS];
	int i;
	for(i = 0; i < NUM_INTS; ++i) {
		data[i] = i;
	}
	rd_done = true;
	mem_stream_init(mem_out, N_STREAMS);

	// Send the same data to the all streams
	int *shared_buf_pois[N_STREAMS];
	for (int i = 0 ; i < N_STREAMS; i++) {
		shared_buf_pois[i] = data;
	}

	int channel = 0;
	int remain_bytes, send_bytes;
	i = 1;
	do {
		remain_bytes = NUM_INTS*4 - ((long)shared_buf_pois[channel] - (long)data);
		if (remain_bytes) {
			send_bytes = channel_write(mem_out, shared_buf_pois[channel], remain_bytes, channel);
			shared_buf_pois[channel] = shared_buf_pois[channel] + send_bytes/sizeof(int);
			printf("TID{{{TID}}} after Write Operation remain_bytes=%d send_bytes=%d shared_buf_pois[%d]=%ld\n", remain_bytes, send_bytes, channel, (long)shared_buf_pois[channel]);
			// Should not be less then 0
			if ( (remain_bytes - send_bytes) <= 0 ) {
				printf("TID{{{TID}}} channel=%d sent \n", channel);
			}
		}
		channel = ((channel + 1) >= N_STREAMS) ? 0 : channel + 1;
		++i;

	} while(1);
}

void streams_to_ints (vsi::device<int> &mem_input) {
	while( !rd_done ) { };
	int data[NUM_INTS];
	int i, j;
	std::vector <int> data_remain_bytes;
	// Tail pointer of data
	std::vector <int *> data_poi;
	// Initialization of output data
	for (i = 0 ; i < N_STREAMS; i++) {
		data_remain_bytes.push_back(NUM_INTS*4);
		data_poi.push_back(data);
	}

	// program the registers of stream to mm IP
	mem_stream_init(mem_input, N_STREAMS);
	int channel = 0;
	i = 0;
	while(1) {
		++i;
		printf("TID{{{TID}}} Read Iteration = %d data_remain_bytes[%d]=%d\n", i, channel, data_remain_bytes[channel]);
		if(data_remain_bytes[channel]) {
			// Reading data if data are available.
			int read_bytes = channel_read(mem_input, data_poi[channel], data_remain_bytes[channel], channel);
			data_poi[channel] += read_bytes/sizeof(int);
			data_remain_bytes[channel] -= read_bytes;

			// Output data if completed.
			if( data_remain_bytes[channel] == 0) {
				printf("TID{{{TID}}} channel %d read done! \n", channel);
				for(j = 0; j < NUM_INTS; ++j){
					printf("data number %d : %d ", j, data[j]);
				}
				printf("\n");
			}
		}

		// Move to the next channel
		channel++;
		if (channel >= N_STREAMS) {
			// Check all -- if any of them not complete need some data
			bool have_unprocess = false;
			for(auto remain_bytes: data_remain_bytes) {
				if(remain_bytes) {
					have_unprocess = true;
					break;
				}
			}

			if (!have_unprocess) {
				printf("TID{{{TID}}} processing done. Going to sleep.\n");
				while(1){
					 sleep(10);
				}
			}

			channel = 0 ;
		}

	} // stall forever
}

void float_to_streams (vsi::device<int> &mem_out) {

	float data[]={4207837.000000, 4207839.500000, 4207841.500000, 4207844.000000,
			   4207846.000000, 4207848.500000, 4207850.500000, 4207852.500000,
               4207762.500000, 4207765.000000, 4207767.500000, 4207769.500000,
                4207771.500000, 4207774.000000, 4207776.000000, 4207778.000000};
	int i;

	rd_done = true;
	mem_stream_init(mem_out, N_STREAMS);

	// Send the same data to the all streams
	int *shared_buf_pois[N_STREAMS];
	for (int i = 0 ; i < N_STREAMS; i++) {
		shared_buf_pois[i] = (int *)data;
	}

	int channel = 0;
	int remain_bytes, send_bytes;
	i = 1;
	do {
		remain_bytes = NUM_FLOATS*4 - ((long)shared_buf_pois[channel] - (long)data);
		if (remain_bytes) {
			send_bytes = channel_write(mem_out, shared_buf_pois[channel], remain_bytes, channel);
			shared_buf_pois[channel] = shared_buf_pois[channel] + send_bytes/sizeof(int);
			printf("TID{{{TID}}} after Write Operation remain_bytes = %d send_bytes=%d shared_buf_pois[%d]=%ld\n", remain_bytes, send_bytes, channel, (long)shared_buf_pois[channel]);
			// Should not be less then 0
			if ( (remain_bytes - send_bytes) <= 0 ) {
				printf("TID{{{TID}}} channel=%d sent \n", channel);
			}
		}
		channel = ((channel + 1) >= N_STREAMS) ? 0 : channel + 1;
		++i;

	} while(1);
}

void streams_to_floats (vsi::device<int> &mem_input) {
	while( !rd_done ) { };
	int data[NUM_FLOATS];
	int i, j;
	std::vector <int> data_remain_bytes;
	// Tail pointer of data
	std::vector <int *> data_poi;
	// Initialization of output data
	for (i = 0 ; i < N_STREAMS; i++) {
		data_remain_bytes.push_back(NUM_FLOATS*4);
		data_poi.push_back(data);
	}

	// program the registers of stream to mm IP
	mem_stream_init(mem_input, N_STREAMS);
	int channel = 0;
	i = 0;
	while(1) {
		++i;
		printf("TID{{{TID}}} Read Iteration=%d data_remain_bytes[%d]=%d\n", i, channel, data_remain_bytes[channel]);
		if(data_remain_bytes[channel]) {
			// Reading data if data are available.
			int read_bytes = channel_read(mem_input, data_poi[channel], data_remain_bytes[channel], channel);
			data_poi[channel] += read_bytes/sizeof(int);
			data_remain_bytes[channel] -= read_bytes;

			// Output data if completed.
			if( data_remain_bytes[channel] == 0) {
				printf("TID{{{TID}}} channel %d read done! \n", channel);
				for(j = 0; j < NUM_FLOATS; ++j){
					printf("data number %d : %f \t", j, *(float *)&data[j]);
				}
				printf("\n");
			}
		}

		// Move to the next channel
		channel++;
		if (channel >= N_STREAMS) {
			// Check all -- if any of them not complete need some data
			bool have_unprocess = false;
			for(auto remain_bytes: data_remain_bytes) {
				if(remain_bytes) {
					have_unprocess = true;
					break;
				}
			}

			if (!have_unprocess) {
				printf("TID{{{TID}}} processing done. Going to sleep.\n");
				while(1){
					 sleep(10);
				}
			}

			channel = 0 ;
		}

	} // stall forever
}
