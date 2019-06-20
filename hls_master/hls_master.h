#ifndef _HLS_MASTER_
#define _HLS_MASTER_
#include <ap_int.h>
typedef struct {
	ap_int<32> data;
	ap_int<1>  last;
} axis_dl;

#endif
