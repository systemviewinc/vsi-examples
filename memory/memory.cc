#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "mem_test.h"
#ifndef __VSI_HLS_SYN__
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
		printf("Write to memory complete {{{TID}}}\n");
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
		printf("Read Complete {{{TID}}}\n");
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
#endif
/**
 * @brief read from a file and write to a stream then go to sleep
 *
 * @param out_s : output stream
 */
void read_file_send(hls::stream<int> &out_s)
{
	// read from a file & write to a stream
	FILE *f;
	if ((f = fopen("./infile.txt","r")) == NULL) {
		perror("Can not open inputfile 'infile.txt'\n");
	}
	printf("Starting read & send\n");
	while (!feof(f)) {
		int fdata, rv;
		if ((rv = fread(&fdata,sizeof(fdata),1,f)) == 1) {
			out_s.write(fdata);
		}
	}
	printf("closing \n");
	fclose(f);
        printf("done read_send going to sleep");
	while(1) sleep(10);
}

/**
 * @brief read from a stream & write to a file
 *
 * @param in_s : input stream
 */
void recv_write_file(hls::stream<int> &in_s)
{
	FILE *f;
	if ((f = fopen("./outfile.txt","w")) == NULL) {
		perror("Can not open outputfile 'outfile.txt'\n");
	}
	while(1) {
		// wait for data to arrive & write to file
		int fdata = in_s.read() ; // blocking read
		fwrite(&fdata,sizeof(fdata),1,f);
		fflush(f);
	}
}

/**
 * @brief process data in batches of 16 integers
 *
 * @param in_s
 * @param out_s
 */
void process_data(hls::stream<int> & in_s, hls::stream<int> & out_s) {
	// process in chunks of 16 integers
	int fdata[16];
	printf(" gathering 16 integers\n");
	for (int i = 0 ; i < sizeof(fdata)/sizeof(fdata[0]); i++) {
		fdata[i] = in_s.read();
	}
	printf(" write them to the output\n");
	for (int i = 0 ; i < sizeof(fdata)/sizeof(fdata[0]); i++) {
		out_s.write(fdata[i]);
	}
}
