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

void _bnchmrk_mem_wr(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out, int threadNum)
{
	const char *numThreadsStr = std::getenv("num_threads");
	setbuf(stdout, NULL); // fixes problem with prints not getting flushed into nohup output logs
	int numThreads;
	if(numThreadsStr){
		sscanf(numThreadsStr, "%d", &numThreads);
	}
	else{
		printf("ERROR: num_threads env var improperly set\n");
		exit(-1);
	}
	if(threadNum > numThreads){
		// numThreads controls how many threads run.
		// This thread won't run if it's out of range.
		while (1) sleep(600);
	}

	const char *dmaSzStr = std::getenv("dma_pkt_sz");
	const char *doMmapStr = std::getenv("do_mmap");
	int dmaSz, doMmap;
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
		printf("ERROR: do_mmap env var improperly set\n");
		exit(-1);
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
			printf("%d %d %d w %lu %lu\n", numThreads, dmaSz, threadNum, w_time, t_bytes);
			while (1) sleep(600);
		}
		// if (offset == (1024*1024*1024)) offset = 0;
		// else offset += 4096;
	}
}

void _bnchmrk_mem_rd(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out, int threadNum)
{
	const char *numThreadsStr = std::getenv("num_threads");
	int numThreads;
	if(numThreadsStr){
		sscanf(numThreadsStr, "%d", &numThreads);
	}
	else{
		printf("ERROR: num_threads env var improperly set\n");
		exit(-1);
	}
	if(threadNum > numThreads){
		// numThreads controls how many threads run.
		// This thread won't run if it's out of range.
		while (1) sleep(600);
	}

	const char *dmaSzStr = std::getenv("dma_pkt_sz");
	const char *doMmapStr = std::getenv("do_mmap");
	int dmaSz, doMmap;
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
		printf("ERROR: do_mmap env var improperly set\n");
		exit(-1);
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
			printf("%d %d %d r %lu %lu\n", numThreads, dmaSz, threadNum, r_time, t_bytes);
		}
		// if (offset == (1024*1024*1024)) offset = 0;
		// else offset += 4096;
	}
}

void bnchmrk_mem_wr_thread1(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 1);
}

void bnchmrk_mem_wr_thread2(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 2);
}

void bnchmrk_mem_wr_thread3(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 3);
}

void bnchmrk_mem_wr_thread4(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 4);
}

void bnchmrk_mem_wr_thread5(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 5);
}

void bnchmrk_mem_wr_thread6(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 6);
}

void bnchmrk_mem_wr_thread7(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 7);
}

void bnchmrk_mem_wr_thread8(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_wr(mem, ctl_in, ctl_out, 8);
}

void bnchmrk_mem_rd_thread1(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 1);
}

void bnchmrk_mem_rd_thread2(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 2);
}

void bnchmrk_mem_rd_thread3(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 3);
}

void bnchmrk_mem_rd_thread4(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 4);
}

void bnchmrk_mem_rd_thread5(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 5);
}

void bnchmrk_mem_rd_thread6(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 6);
}

void bnchmrk_mem_rd_thread7(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 7);
}

void bnchmrk_mem_rd_thread8(vsi::device &mem, hls::stream<int> &ctl_in, hls::stream<int> &ctl_out)
{
	_bnchmrk_mem_rd(mem, ctl_in, ctl_out, 8);
}

#endif
