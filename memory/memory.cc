#include <stdio.h>
#include <string.h>
#include "mem_test.h"

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

/** 
 * @brief write a patter to memory and tell another process when write is
 * 	  complete and wait for ack from the other process.
 * @param mem 		: memory elemnt to write to
 * @param ctl_in 	: control in
 * @param ctl_out 	: control out
 */
void mem_write(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	char val[4096] ;
	char wval = 'a';
	while (1) {
		printf("Writing to memory\n");
		memset(val,wval,sizeof(val));
		mem.pwrite(val,sizeof(val),0); // write value at offset 0
		printf("Write to memory complete\n");
		ctl_out.write(1); // tell waiting thread write is complete
		ctl_in.read();	  // wait for other process to continue
		if (wval != 'z') wval++;
		else wval = 'a';
	}
}

/** 
 * @brief read memory and check if the patter matches
 * 
 * @param mem 
 * @param ctl_in 
 * @param ctl_out 
 */
void mem_read(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	char val[4096] ;
	char wval = 'a';
	while (1) {
		ctl_in.read(); // wait to proceed
		mem.pread(val,sizeof(val),0); // read from offset 0
		printf("Read Complete\n");
		for (int i = 0; i < sizeof(val); i++) {
			if (val[i] != wval) {
				printf("ERROR: mismatch expected '0x%x' got '0x%x'\n",wval,val[i]);
				exit(-1);
			}
		}
		ctl_out.write(1); // tell waiting thread to proceed
	}
}
