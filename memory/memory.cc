#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mem_test.h"

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
	int offset = 0 ;
	while (1) {
		printf("Writing to memory\n");
		memset(val,wval,sizeof(val));
		mem.pwrite(val,sizeof(val),offset); 	// write value at offset
		printf("Write to memory complete\n");
		ctl_out.write(1); 			// tell waiting thread write is complete
		//usleep(500);
		ctl_in.read();	  			// wait for other process to continue
		if (wval != 'z') wval++;
		else {
			printf("Test Complete thread going to sleep\n");
			while (1) sleep(10);
		}
		if (offset == (1024*1024*1024)) offset = 0;
		else offset += 4096;
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
	int offset = 0 ;
	while (1) {
		ctl_in.read(); 			// wait to proceed
		mem.pread(val,sizeof(val),offset); // read from offset 
		printf("Read Complete\n");
		for (int i = 0; i < sizeof(val); i++) {
			if (val[i] != wval) {
				printf("ERROR: mismatch expected '0x%x' got '0x%x'\n",wval,val[i]);
				exit(-1);
			}
		}
		ctl_out.write(1); // tell waiting thread to proceed
		if (wval != 'z') wval++;
		else wval = 'a';
		if (offset == (1024*1024*1024)) offset = 0;
		else offset += 4096;
	}
}

void mem_read_write(vsi::device &mem) {
	char val[4096] ;
	char wval = 'a';
	int offset = 0 ;
	while (1) {
		mem.pwrite(val,sizeof(val),offset); // write to  offset
		if (wval != 'z') wval++;
		else wval = 'a';
		if (offset == (1024*1024*1024)) offset = 0;
		else offset += 4096;
		sleep(1);
	}	
}
