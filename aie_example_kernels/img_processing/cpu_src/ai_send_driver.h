#ifndef _AI_SEND_DRIVER_H_
#define _AI_SEND_DRIVER_H_

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include <string.h>
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include <unistd.h>
#endif

#define N_STREAMS 1


// Channel mm size
#define CHANNEL_SPAN (8*1024)
#define DATA_BASE_OFFSET  0x20000
// Number of control registers per channel
#define CTR_REG_NUM (4)
// stream2mm mm2ai register map
#define LOW_RANGE_REG   0
#define HIGHT_RANGE_REG 1
#define EFULL_REG       2
#define LEVEL_REG       3
//
#define CH_LOW_RANGE_REG(CH_NUM)   ((CH_NUM*CTR_REG_NUM + LOW_RANGE_REG) << 2)
#define CH_HIGHT_RANGE_REG(CH_NUM) ((CH_NUM*CTR_REG_NUM + HIGHT_RANGE_REG) << 2)
#define CH_EFULL_REG(CH_NUM)       ((CH_NUM*CTR_REG_NUM + EFULL_REG) << 2)
#define CH_LEVEL_REG(CH_NUM)       ((CH_NUM*CTR_REG_NUM + LEVEL_REG) << 2)

#define CH_DATA_ADDR(CH_NUM)       ( DATA_BASE_OFFSET + (CH_NUM*CHANNEL_SPAN << 2) )

// Function to initialize mm2stream & stream2mm IP
void mem_stream_init(vsi::device &mem, unsigned int channel_number);
void get_channel_level( vsi::device &mem,
                        unsigned int *efull,
                        unsigned int *level,
                        unsigned int channel_number);
void pull_remain(vsi::device &mem, int stream_number);

// void full(vsi::device &mem_out, int channel){};
// void empty(vsi::device &mem_out, int channel){};
int channel_read(vsi::device &mem, int *buf, int size, int channel);
int channel_write(vsi::device &mem, int *buf, int size, int channel);

#endif
