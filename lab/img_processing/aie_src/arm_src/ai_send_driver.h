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

void printbuf(ofstream *logfile, int *buf, int len);
void blocked_write (vsi::device<int> &mem_out, int* buf, int size, int addr);
void blocked_read (vsi::device<int> &mem_out, int* buf, int size, int addr);
void hash_compute(int *buf, int size);
void aximm_to_streams (vsi::device<int> &mem_out);
void streams_to_aximm (vsi::device<int> &mem_out);

#endif
