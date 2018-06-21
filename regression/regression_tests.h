#ifndef REGRESSION_TEST_H
#define REGRESSION_TEST_H
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include "quick_sort.h"

#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <hls_stream.h>
#endif

#define DATA_WIDTH 32
#define USER_WIDTH 8


template<int D>
struct test_last_type {
	ap_uint<D> data;
	ap_uint<1> last;
};


template<int D>
struct test_user_last_type {
	ap_uint<D> data;
	ap_uint<1> last;
	ap_uint<USER_WIDTH> user;
	ap_uint<D/8> keep;
	ap_uint<1> id;
};

#endif
