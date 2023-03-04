#include <mutex>
#include <shared_mutex>
#include <unistd.h>
#include <string.h>
#include <vsi_device.h>
#include <rdma_helper.h>
#include "rdma.h"
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#define _RDMA_DRIVER_

#define MEM_SIZE 8*512*1024

/* Include RDMA programs */

unsigned int rdma_mem2aie [] = {
#include "rdma_mem2aie.h"
};
size_t rdma_mem2aie_sz = sizeof(rdma_mem2aie);

unsigned int rdma_aie2mem [] = {
#include "rdma_aie2mem.h"
};
size_t rdma_aie2mem_sz = sizeof(rdma_aie2mem);

/* Define RDMA drivers */

#define TOT_RDMA_PROGS_REGISTERED 2

void rdma_mem2aie_driver(vsi::device<int> &rdma_control, vsi::device<int> &program_mem) {
	std::string rdma_name("mm2aie");
	register_rdma_prog(rdma_name, 0, rdma_mem2aie, rdma_mem2aie_sz);
	int err = process_rdma_state(rdma_name, rdma_control, program_mem);
	/* Function above will only return with error, handle below */
	printf("%s FAILED: Error code = %d\n", __FUNCTION__, err);
	exit(1);
}

void rdma_aie2mem_driver(vsi::device<int> &rdma_control, vsi::device<int> &program_mem) {
	std::string rdma_name("aie2mm");
	register_rdma_prog(rdma_name, 0, rdma_aie2mem, rdma_aie2mem_sz);
	int err = process_rdma_state(rdma_name, rdma_control, program_mem);
	/* Function above will only return with error, handle below */
	printf("%s FAILED: Error code = %d\n", __FUNCTION__, err);
	exit(1);
}

#define DMEM_SIZE 65536
//#define LOAD_SIZE DMEM_SIZE
#define LOAD_SIZE 4096 // goes faster for co-simulation
#define RD_CHUNK_SZ 1024 // number of bytes in each pread from ddr
#define XK_DAT_SZ 32 // number of uint16's in each XK IO buff
#define XK_TOT_OUT 5376 // total num bytes expected from XK run
#define XK_TOT_OUT_VALS XK_TOT_OUT/2 // 2 bytes per xk val
static uint16_t mem_init[DMEM_SIZE];
// input data to xk
static uint16_t xk_in_dat[XK_DAT_SZ] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14};
// expected output data from xk
static uint16_t xk_out_dat[XK_DAT_SZ] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	2, 0, 6, 4, 10, 8, 14, 12, 18, 16, 22, 20, 26, 24, 30, 28};
void load_validate_memory(vsi::device<int> &mem)
{
	set_tot_rdma_registrations(TOT_RDMA_PROGS_REGISTERED);
	int i;
	uint16_t mi;
	uint16_t ri = 0;

	printf("%s: Loading AIE test data into DDR...\n", __FUNCTION__);

	int offset = 0;
	char *sdata_p = (char *)&mem_init[0];
	for(mi = 0 ; mi < DMEM_SIZE-1; ++mi){
		mem_init[mi] = xk_in_dat[ri];
		++ri;
		if(ri == XK_DAT_SZ){
			ri = 0;
		}
	}
	while (sdata_p < &mem_init[LOAD_SIZE]) {
		//printf("%s: loading chunk...\n", __FUNCTION__);
		mem.pwrite(sdata_p, 4096, offset);
		printf("%s: Sent chunk, offset=%d\n",__FUNCTION__, offset);
		sdata_p += 4096;
		offset  += 4096;
	}

	printf("%s: Running RDMAs then validating data.\n", __FUNCTION__);

	int ret, val, cmpVal, byteCount, errCount, rd_loop_iters, rd_loop_iter_count, xkoBuff_i, valCount;
	uint8_t readDat[RD_CHUNK_SZ];

	ret = run_rdma_grp(0);
	while(ret){ // if ret not zero, RDMA group is not ready to run
		printf("%s: WARNING run_rdma_grp(%d) returned %d\n", __FUNCTION__, 0, ret);
		sleep(1);
		ret = run_rdma_grp(0);
	}

	// loop through and read the area of memory to which rdma trasfers aie output
	byteCount = 0;
	errCount = 0;
	valCount  = 0;
	xkoBuff_i = 0;
	rd_loop_iters = 6;
	for(offset = 0x20000; offset < 0x20000 + RD_CHUNK_SZ*rd_loop_iters; offset += RD_CHUNK_SZ){
		ret = mem.pread(readDat, RD_CHUNK_SZ, offset);
		byteCount += RD_CHUNK_SZ;
		printf("%s: read %d bytes\n", __FUNCTION__, byteCount);
		for(i = 0; i < RD_CHUNK_SZ/2; ++i){
			// get/print data
			val = *(((uint16_t*)readDat)+i);
			printf("%d:", val);
			// error checking
			++valCount;
			if(val != (int)xk_out_dat[xkoBuff_i] && valCount <= XK_TOT_OUT_VALS){
				++errCount;
				printf("<ERROR!:");
			}
			++xkoBuff_i;
			if(xkoBuff_i == XK_DAT_SZ){
				xkoBuff_i = 0;
			}
		}
		printf("\n");
	}

	printf("%s: read back data complete\n",__FUNCTION__);
	if(errCount){
		printf("%s ERROR: corrupt data read back from DDR!\n",__FUNCTION__);
	}
	else{
		printf("%s SUCCESS: valid data read back from DDR!\n",__FUNCTION__);
	}

	exit(0); // Exit sw application.
}
