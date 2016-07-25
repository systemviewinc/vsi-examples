#include <stdio.h>
#include <string.h>
#include "memory.h"

void vsi_memory(int in_arr[1024], int *out_mem, int out_arr[1024])
{
	int i_buff[1024];
	int i;
	// copy from memory
	memcpy(i_buff,out_mem,sizeof(i_buff)); 
	// perform operation
	for (i = 0 ; i < 1024; i++) {
#pragma HLS PIPELINE II=1
		i_buff [i] += in_arr[i];
		out_arr[i] = i_buff[i];
	}
	// copy back to external memory
	memcpy(out_mem,i_buff,sizeof(i_buff));
}


/** 
 * @brief receive data from "world" copy to memory and start next
 * 
 * @param in_arr 	- input data
 * @param out_mem 	- pointer to external memory
 * @param start 	- start to next
 * @param resp 		- receive response
 */
void vsi_memory_ctl(int in_arr[1024], 
		    int *out_mem, 
		    hls::stream<ap_uint<32> > &start, 
		    hls::stream<ap_uint<32> > &resp)
{
	int i_buff[1024];
	int i;

	// perform operation
	for (i = 0 ; i < 1024; i++) {
#pragma HLS PIPELINE II=1
		i_buff [i] = in_arr[i];
	}
	// copy to external memory
	memcpy(out_mem,i_buff,sizeof(i_buff));

	ap_uint<32> w = 1;

	// tell next process something in memory
	start.write(w); 

	// wait for response
	ap_uint<32> r = resp.read(); 
}


/** 
 * @brief wait for data process data in the memory & say done
 * 
 * @param start 
 * @param mem 
 * @param sdone 
 */
void vsi_process_data(hls::stream<ap_uint<32> > &start,
		      hls::stream<ap_uint<32> > &sdone,
		      int *mem,
		      int out_arr[1024])
{
	ap_uint<32> s = start.read(); // wait for data
	
	// process the data
	for (int i = 0 ; i < 1024; i++) {
#pragma HLS PIPELINE II=1
		out_arr[i] = (mem[i] += 100);
	}

	// tell we are done
	ap_uint<32> w= 1;
	sdone.write(w);
}

/** 
 * @brief just a pass through
 * 
 * @param in_arr 
 * @param out_arr 
 */
void pass_thru(int in_arr[1024], int out_arr[1024]) 
{
	for (int i =0 ; i < 1024; i++) 
#pragma HLS PIPELINE II=1
		out_arr[i] = in_arr[i];
}
