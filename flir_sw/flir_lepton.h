#ifndef FLIR_LEPTON_H_
#define FLIR_LEPTON_H_

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include <limits.h>
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#endif
#include "stream_mux.h"
#define DATA_WIDTH 16
#define NCOLS 80
#define NROWS 60 
#define FRAME_SIZE (sizeof(uint16_t)*NCOLS*NROWS)


#endif
