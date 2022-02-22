#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstddef>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <time.h>

#include <chrono>

using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#endif

// use internal program memory
#undef INTERNAL_MEM
#undef SUPPORT_MSG
#undef WAIT_PRESS_KEY

#define TEST_NUM 16
#define MAX_CHUNK 16
#define CHUNK_SIZE (1 * 256)
#define INITIAL_CHUNK_SIZE (1 * CHUNK_SIZE)

enum Speedunits { bps = 1, Kbps, Mbps, Gbps, Tbps, Null };

struct benchTestData{
	bool restart = false;
	bool readProc = false;
	bool writeProc = false;
	bool inprocess = true;

	unsigned int testID = 0;
	unsigned int bufSize = INITIAL_CHUNK_SIZE;

	unsigned int writeID = 0;
	unsigned int readID = 0;

	long long int writetime = -1;
	long long int readtime = -1;

	bool memallocerror = false;
	char * writeBuf;
	char * readBuf;
	
	char ch;
};

#endif
