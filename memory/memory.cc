#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "mem_test.h"

#ifndef __VSI_HLS_SYN__
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

#define MEM_SIZE 4096
#define BOARD_MEM_SIZE 4096
#define END_CHAR 'z'

/**
 * @brief write a patter to memory and tell another process when write is
 * 	  complete and wait for ack from the other process.
 * @param mem 		: memory elemnt to write to
 * @param ctl_in 	: control in
 * @param ctl_out 	: control out
 */
void mem_write(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	char val[MEM_SIZE] ;
	char wval = 'a';
	int offset = 0 ;
	std::chrono::duration<double,std::milli> w_time = std::chrono::duration<double,std::milli>::zero();
	unsigned long t_bytes = 0;
	while (1) {
		std::cout << "Writing to memory\n";
		memset(val,wval,sizeof(val));
		// time the write
		auto t_start = std::chrono::high_resolution_clock::now();
		mem.pwrite(val,sizeof(val),offset); 	// write value at offset
		auto t_end   = std::chrono::high_resolution_clock::now();
		printf("Write to memory complete {{{TID}}}\n");
		w_time  += (t_end - t_start);
		t_bytes += MEM_SIZE;
		std::cout << "Writing complete\n";

		ctl_out.write(1); 			// tell waiting thread write is complete
		//usleep(500);
		ctl_in.read();	  			// wait for other process to continue
		if (wval != END_CHAR) wval++;
		else {
			std::cout << "Test Complete thread going to sleep "
				  << "wrote " << t_bytes << " in "
				  << w_time.count() << " ms\n";
			while (1) sleep(10);
		}
		if (offset == (1024*1024*1024)) offset = 0;
		else offset += 4096;
	}
}

/**
 * @brief read memory and check if the pattern matches
 *
 * @param mem
 * @param ctl_in
 * @param ctl_out
 */
void mem_read(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	char val[MEM_SIZE] ;
	char wval = 'a';
	int offset = 0 ;
	std::chrono::duration<double,std::milli> r_time = std::chrono::duration<double,std::milli>::zero();
	unsigned long t_bytes = 0;
	while (1) {
		ctl_in.read(); 			// wait to proceed
		mem.pread(val,sizeof(val),offset); // read from offset
		printf("Read Complete {{{TID}}}\n");
		// time the read
		auto t_start = std::chrono::high_resolution_clock::now();
		mem.pread(val,sizeof(val),offset); // read from offset
		auto t_end   = std::chrono::high_resolution_clock::now();
		r_time  += (t_end - t_start);
		t_bytes += MEM_SIZE;

		std::cout << "Read Complete\n";
		for (int i = 0; i < sizeof(val); i++) {
			if (val[i] != wval) {
				printf("ERROR: mismatch expected '0x%x' got '0x%x'\n",wval,val[i]);
				exit(-1);
			}
		}
		ctl_out.write(1); // tell waiting thread to proceed
		if (wval != END_CHAR) wval++;
		else {
			std:: cout << "Test Complete thread going to sleep "
				   << "read " << t_bytes << " in "
				   << r_time.count() << " ms\n";
		}
		if (offset == (1024*1024*1024)) offset = 0;
		else offset += 4096;
	}
}
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
 * @brief write a patter to memory and tell another process when write is
 * 	  complete and wait for ack from the other process.
 * @param mem 		: memory elemnt to write to
 * @param ctl_in 	: control in to start testing
 */
void mem_write_read(vsi::device &mem, hls::stream<int> &ctl_in)
{
	char val[MEM_SIZE] ;
	char wval = 'a';
	int offset = 0;

	std::chrono::duration<double,std::milli> w_time = std::chrono::duration<double,std::milli>::zero();
	std::chrono::duration<double,std::milli> r_time = std::chrono::duration<double,std::milli>::zero();

	unsigned long t_bytes = 0;
	while(ctl_in.empty()){
		sleep(1);
	}
	while(!ctl_in.empty()){
		ctl_in.read();	  			// read out the whole input stream
	}
	while (1) {
		std::cout << "Writing to memory\n";
		memset(val,wval,sizeof(val));
		// time the write
		printf("Write to memory complete {{{TID}}}\n");
		auto t_start = std::chrono::high_resolution_clock::now();
		mem.pwrite(val,sizeof(val),offset); 	// write value at offset
		auto t_end   = std::chrono::high_resolution_clock::now();
		w_time  += (t_end - t_start);
		t_bytes += MEM_SIZE;
		std::cout << "Writing complete\n";

		std::cout << "Reading from memory\n";
		t_start = std::chrono::high_resolution_clock::now();
		mem.pread(val,sizeof(val),offset); // read from offset
		t_end   = std::chrono::high_resolution_clock::now();
		printf("Read Complete {{{TID}}}\n");
		// time the read

		r_time  += (t_end - t_start);
		t_bytes += MEM_SIZE;

		std::cout << "Read Complete\n";

		if (wval != END_CHAR) wval++;
		else {
			std::cout << "Test Complete thread going to sleep "
				  << "wrote " << t_bytes << " in "
				  << w_time.count() << " ms\n";
		  std::cout << "Test Complete thread going to sleep "
				 << "read " << t_bytes << " in "
				 << r_time.count() << " ms\n";
				 break;
		}
		offset += sizeof(val);
		if (offset >= BOARD_MEM_SIZE) offset = 0;
	}
}
#endif

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

void mem_arr(char ar[1024])
{
	for (int i = 0 ; i < 1024; i++)
		ar[i] = i;
}

/* the following are for testing connection to vsi_mem */

/* create and send some data to the array */
void mem_write_array(hls::stream<int> &done,
		     int mem_array[16][1024]) {
	static int first = 0;
	if (first) {
		done.write(1); // start the read
		printf("Started read going to sleep\n");
		while(1) sleep(1); // sleep for ever
	}
	for (int i = 0 ; i < 16; i++) {
		for (int j = 0 ; j < 1024; j++) {
			mem_array[i][j]= i*j;
		}
	}
	printf("Complete write\n");
	first = 1;
}

void mem_read_array(hls::stream<int> &start,
		    int mem_array[16][1024]) {
	int s = start.read();
	printf("Read started\n");
	for (int i = 0 ; i < 16; i++) {
		for (int j = 0 ; j < 1024; j++) {
			if (mem_array[i][j] != i*j)
				printf("Error: did not match mem_array[%d][%d] got %d, expected %d\n",
				       i,j, mem_array[i][j], i*j);
		}
	}
	printf("Matched\n");
	while(1) sleep(1);

}
