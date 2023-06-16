#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "benchmark_memory.h"

#ifndef __VSI_HLS_SYN__
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

#define END_CHAR '~'
#define BNCHMRK_MAX_DMA_SZ 131072
#define ITERATIONS 6

/* Master control function/block for all threads and run iterations. */
void master(hls::stream<int> &t1en, hls::stream<int> &t1done)
{
	/* num_threads is an environment variable set by the user to control the maximum number of w/r threads
	that will run. Write/read functions are sequentially locked, so considered one thread each. */
	const char *numThreadsStr = std::getenv("num_threads");
	setbuf(stdout, NULL); // fixes problem with prints not getting flushed into nohup output logs
	int numThreads;
	if(numThreadsStr){
		sscanf(numThreadsStr, "%d", &numThreads);
		if(numThreads < 1 || numThreads > 8){
			printf("ERROR: num_threads invalid value\n");
			exit(-1);
		}
	}
	else{
		printf("ERROR: num_threads env var improperly set\n");
		exit(-1);
	}

	const char *fromOneStr = std::getenv("from_one");
	int fromOne;
	if(fromOneStr){
		sscanf(fromOneStr, "%d", &fromOne);
	}
	else{
		fromOne = 0;
	}

	int th, i;
	if(fromOne){
		th = 1;
	}
	else{
		th = numThreads;
	}
	/* If fromOne set, this loop will go thru all the thread levels from one upto the numThreads value. */
	/* It succesively runs more conncurrent threads. */
	while(th <= numThreads){
		/* Run several iterations to collect lots of data (more data gives more accurate results). */
		for(i = 0; i < ITERATIONS; ++i){
			/* Thread level control. Each case number will run concurrently that number of threads. */
			switch(th){
				case 1:
					t1en.write(1);
					sleep(3);
					t1done.read();
					break;
			}
			printf("done: thread level %d, iteration %d\n", th, i+1);
		}
		++th;
	}
	exit(0); // exit benchmark executable after all iterations complete
}

/* Core benchmark memory write function. */
void _bnchmrk_mem_wr(vsi::device<int> &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out,
											int threadNum, hls::stream<int> &en)
{
	en.read(); // wait until an enable signal is received from master

	/* Get and check environment variables. */
	const char *numThreadsStr = std::getenv("num_threads");
	const char *dmaSzStr = std::getenv("dma_pkt_sz");
	const char *doMmapStr = std::getenv("do_mmap");
	const char *fromOneStr = std::getenv("from_one");
	int numThreads, dmaSz, doMmap, fromOne;
	if(numThreadsStr){
		sscanf(numThreadsStr, "%d", &numThreads);
	}
	else{
		printf("ERROR: num_threads env var improperly set\n");
		exit(-1);
	}
	if(dmaSzStr){
		sscanf(dmaSzStr, "%d", &dmaSz);
	}
	else{
		printf("ERROR: dma_pkt_sz env var improperly set\n");
		exit(-1);
	}
	if(doMmapStr){
		sscanf(doMmapStr, "%d", &doMmap);
	}
	else{
		//printf("ERROR: do_mmap env var improperly set\n");
		//exit(-1);
		doMmap = 0;
	}
	char fromOneNote[4] = "nfo";
	if(fromOneStr){
		sscanf(fromOneStr, "%d", &fromOne);
		if(fromOne){
			strcpy(fromOneNote, "fo");
		}
	}
	else{
		strcpy(fromOneNote, "nfo");
	}
	//printf("TID{{{TID}}} bmWr dmaSz=%d doMmap=%d numThreads=%d threadNum=%d\n", dmaSz, doMmap, numThreads, threadNum);

	char val[BNCHMRK_MAX_DMA_SZ];
	char wval = '!';
	int offset = 0 ;
	unsigned long w_time = 0;
	unsigned long t_bytes = 0;
	unsigned long dif;
	std::chrono::high_resolution_clock::time_point t1, t2;
	if(doMmap){
		mem.mmap();
	}
	while (1) {
		memset(val, wval, dmaSz);
		// time the write
		t1 = std::chrono::high_resolution_clock::now();
		mem.pwrite(val, dmaSz, offset); 	// write value at offset
		t2 = std::chrono::high_resolution_clock::now();
		dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		w_time += dif;
		t_bytes += dmaSz;
		//printf("TID{{{TID}}} Write iteration complete %lu\n", dif);
		//printf("TID{{{TID}}} Write iteration complete %lu\n", t_bytes);

		ctl_out.write(1); 			// tell waiting thread write is complete
		//usleep(500);
		ctl_in.read();	  			// wait for other process to continue
		if (wval != END_CHAR) wval++;
		else {
			printf("%d %d %d w %lu %lu %s\n", numThreads, dmaSz, threadNum, w_time, t_bytes, fromOneNote);
			break;
		}
		// if (offset == (1024*1024*1024)) offset = 0;
		// else offset += 4096;
	}
}

/* Core benchmark memory read function. */
void _bnchmrk_mem_rd(vsi::device<int> &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out,
											int threadNum, hls::stream<int> &done)
{
	/* Get and check environment variables. */
	const char *numThreadsStr = std::getenv("num_threads");
	const char *dmaSzStr = std::getenv("dma_pkt_sz");
	const char *doMmapStr = std::getenv("do_mmap");
	const char *fromOneStr = std::getenv("from_one");
	int numThreads, dmaSz, doMmap, fromOne;
	if(numThreadsStr){
		sscanf(numThreadsStr, "%d", &numThreads);
	}
	else{
		printf("ERROR: num_threads env var improperly set\n");
		exit(-1);
	}
	if(dmaSzStr){
		sscanf(dmaSzStr, "%d", &dmaSz);
	}
	else{
		printf("ERROR: dma_pkt_sz env var improperly set\n");
		exit(-1);
	}
	if(doMmapStr){
		sscanf(doMmapStr, "%d", &doMmap);
	}
	else{
		//printf("ERROR: do_mmap env var improperly set\n");
		//exit(-1);
		doMmap = 0;
	}
	char fromOneNote[4] = "nfo";
	if(fromOneStr){
		sscanf(fromOneStr, "%d", &fromOne);
		if(fromOne){
			strcpy(fromOneNote, "fo");
		}
	}
	else{
		strcpy(fromOneNote, "nfo");
	}
	//printf("TID{{{TID}}} bmRd dmaSz=%d doMmap=%d numThreads=%d threadNum=%d\n", dmaSz, doMmap, numThreads, threadNum);

	char val[BNCHMRK_MAX_DMA_SZ];
	char wval = '!';
	int offset = 0 ;
	unsigned long r_time = 0;
	unsigned long t_bytes = 0;
	unsigned long dif;
	std::chrono::high_resolution_clock::time_point t1, t2;
	if(doMmap){
		mem.mmap();
	}
	while (1) {
		ctl_in.read(); 			// wait to proceed
		// time the read
		t1 = std::chrono::high_resolution_clock::now();
		mem.pread(val, dmaSz, offset); // read from offset
		t2 = std::chrono::high_resolution_clock::now();
		dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		r_time += dif;
		t_bytes += dmaSz;
		//printf("TID{{{TID}}} Read iteration complete %lu\n", dif);
		//printf("TID{{{TID}}} Read iteration complete %lu\n", t_bytes);

		for (int i = 0; i < dmaSz; i++) {
			if (val[i] != wval) {
				printf("ERROR: mismatch expected '0x%x' got '0x%x', i=%d\n", wval , val[i], i);
				exit(-1);
			}
		}
		ctl_out.write(1); // tell waiting thread to proceed
		if (wval != END_CHAR) wval++;
		else {
			printf("%d %d %d r %lu %lu %s\n", numThreads, dmaSz, threadNum, r_time, t_bytes, fromOneNote);
			break;
		}
		// if (offset == (1024*1024*1024)) offset = 0;
		// else offset += 4096;
	}
	done.write(1); // tell master this run is complete
}

/* Wrapper functions. Each execute the same core write/read code but are passed
specific thread identifier numbers so that they can be enabled/monitored. */

void bnchmrk_mem_wr_thread1(vsi::device<int> &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out, hls::stream<int> &en)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 1, en);
}

void bnchmrk_mem_rd_thread1(vsi::device<int> &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out, hls::stream<int> &dn)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 1, dn);
}
#endif
