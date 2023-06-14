#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include <iostream>
#endif

#define N_SAMPLES 16
#define DATA_BIT_WIDTH 32
#define DATA_BYTE_WIDTH DATA_BIT_WIDTH/8


