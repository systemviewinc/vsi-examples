// image_algos.cc ---
//
// Filename: 		image_algos.cc
// Description: 	Contains synthesizable image processing algorithms
// Author: 		Sandeep <sandeep@systemviewinc.com>
// Maintainer:
// Created: 		Wed Jan 10 09:36:47 2018 (-0800)
// Version:
// Package-Requires: ()
// Last-Updated:
//           By:
//     Update #: 0
// URL:
// Doc URL:
// Keywords:
// Compatibility:
//
//

// Commentary:
//
//
//
//

// Change Log:
//
//
//
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.
//
//

// Code:



// ///////////////////////////////////////////////////////////////////
// Synthesizable portion of the file
// ///////////////////////////////////////////////////////////////////


//#include "stream_mux.h"
#include <strings.h>
#include "hls_stream.h"
#include "common/xf_common.h"
#include "common/xf_utility.h"
#include "core/xf_min_max_loc.hpp"
#include "imgproc/xf_median_blur.hpp"
#include "imgproc/xf_gaussian_filter.hpp"
#include "imgproc/xf_mean_shift.hpp"
#include "imgproc/xf_canny.hpp"
#include "imgproc/xf_edge_tracing.hpp"
#include "features/xf_fast.hpp"


//#include "webcam.h"
#include "ap_int.h"


#define _USE_XF_OPENCV
#define FILTER_WIDTH 3

// ///////////////////////////////////////////////////////////////////
// calls the xf::minMaxLoc opencv function to compute the minmax on
// the image it receives on hls::stream<> ins. The output is sent
// on hls::stream<> outs the order is
//   	[0] = min_value
//   	[1] = max_value
// 	[2] = minx loc
//	[3] = miny loc
//	[4] = maxx loc
//	[5] = maxy loc
// ///////////////////////////////////////////////////////////////////
template <int NROWS, int NCOLS, typename T = uint16_t>
void calc_min_max(hls::stream<T> &ins,	hls::stream<int> &outs)
{
#pragma HLS inline self

	uint16_t _min_locx,_min_locy,_max_locx,_max_locy;
	int32_t _min_val = 0xfffff,_max_val = 0, _min_idx = 0, _max_idx = 0;
#ifdef _USE_XF_OPENCV
 	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC1> cam_mat(NROWS,NCOLS);
 	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC1> cam_mat_i(NROWS,NCOLS);
 	for (int idx = 0 ; idx < (NROWS*NCOLS) ; idx++ ) {
#pragma HLS PIPELINE II=1
		T td = ins.read();
		cam_mat.data[idx] = (uint8_t) td;
	}
	//xf::medianBlur <FILTER_WIDTH, XF_BORDER_REPLICATE, XF_8UC1, NROWS, NCOLS,XF_NPPC1> (cam_mat, cam_mat_i);
	xf::minMaxLoc<XF_8UC1,NROWS,NCOLS,XF_NPPC1>(cam_mat, &_min_val, &_max_val, &_min_locx, &_min_locy, &_max_locx, &_max_locy);
	outs.write((int)_min_val);
	outs.write((int)_max_val);
	outs.write((int)_min_locy);
	outs.write((int)_min_locx);
	outs.write((int)_max_locy);
	outs.write((int)_max_locx);
#else  // !_USE_XF_OPENCV
	// copy image into a local buffer
	uint16_t img_data[NROWS*NCOLS];
	#pragma HLS RESOURCE variable=img_data core=RAM_1P_BRAM
 	for (int idy = 0 ; idy < NROWS ; idy++ ) {
		for (int idx = 0 ; idx < NCOLS ; idx++) {
			#pragma HLS PIPELINE II=1

			img_data[(idy*NCOLS)+idx] = ins.read();
		}
	}
	// do median_blur and minmax in the same loop
	for (int idy = 1 ; idy < NROWS-1; idy++) {
		for (int idx = 1; idx < (NCOLS-1); idx++) {
#pragma HLS PIPELINE II=1
			uint32_t data = 0;
			data += (uint32_t)img_data[(idy*NCOLS)+idx];
			data += (uint32_t)img_data[(idy*NCOLS)+idx+1];
			data += (uint32_t)img_data[(idy*NCOLS)+idx-1];
			data += (uint32_t)img_data[((idy+1)*NCOLS)+idx];
			data += (uint32_t)img_data[((idy+1)*NCOLS)+idx+1];
			data += (uint32_t)img_data[((idy+1)*NCOLS)+idx-1];
			data += (uint32_t)img_data[((idy-1)*NCOLS)+idx];
			data += (uint32_t)img_data[((idy-1)*NCOLS)+idx+1];
			data += (uint32_t)img_data[((idy-1)*NCOLS)+idx-1];
			data /= (uint32_t)9;
			if (data < _min_val) {
				_min_val = data;
				_min_locx = idx;
				_min_locy = idy;
			}
			if (data > _max_val) {
				_max_val = data;
				_max_locx = idx;
				_max_locy = idy;
			}
		}
	}
	outs.write((int)_min_val);
	outs.write((int)_max_val);
	outs.write((int)_min_locx);
	outs.write((int)_min_locy);
	outs.write((int)_max_locx);
	outs.write((int)_max_locy);
	//usleep(1000);
#endif
}

// template <int NROWS, int NCOLS, typename T = uint16_t>
// void edge_detect(hls::stream<T> &ins,	hls::stream<T> &outs, float sigma)
// {
// #pragma HLS inline self

//  	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC1> cam_mat(NROWS,NCOLS);
//  	xf::Mat<XF_2UC1,NROWS,NCOLS,XF_NPPC4> cam_mat_i(NROWS,NCOLS);	
// 	xf::Mat<XF_2UC1,NROWS,NCOLS,XF_NPPC32> tmp_mat(NROWS,NCOLS);
// 	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC8> tmp2_mat(NROWS,NCOLS);
	
//  	for (int idx = 0 ; idx < (NROWS*NCOLS) ; idx++ ) {
// #pragma HLS PIPELINE II=1
// 		T td = ins.read();
// 		cam_mat.data[idx] = (uint8_t) td;
// 	}

// 	xf::Canny<FILTER_WIDTH, XF_L1NORM, XF_8UC1, XF_2UC1, NROWS, NCOLS, XF_NPPC1,XF_NPPC4> (cam_mat, cam_mat_i, 20, 54);
// 	tmp_mat.copyTo(cam_mat_i.data); // convert to NPPC32
// 	xf::EdgeTracing<XF_2UC1, XF_8UC1, NROWS, NCOLS, XF_NPPC32, XF_NPPC8> (tmp_mat, tmp2_mat);
// 	cam_mat.copyTo(tmp2_mat.data); // convert to NPPC1
	
// 	for (int idx = 0 ; idx < (NROWS*NCOLS) ; idx++ ) {
// #pragma HLS PIPELINE II=1
// 		outs.write(cam_mat.data[idx]);
// 	}
// }

namespace xf {
template<int ROWS, int COLS>
void xfEdgeTracing_(unsigned long long * _src, unsigned long long *_dst)
{
	xf::xfEdgeTracing((unsigned long long *)_dst,(unsigned long long *)_src,ROWS,COLS);
}
}
template <int NROWS, int NCOLS, typename T = uint64_t>
static void edge_trace (T *src, T *dst)
{
	#pragma HLS interface bram port=src depth=307200
	#pragma HLS interface bram port=dst depth=307200
	xf::xfEdgeTracing_<NROWS,NCOLS>((unsigned long long *)src, (unsigned long long *)dst);
}

template <int NROWS, int NCOLS, typename T = uint8_t>
static void canny_edge(hls::stream<T> &ins, T *outs)
{
	#pragma HLS interface bram port=outs depth=307200
	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC1> cam_mat(NROWS,NCOLS);
 	xf::Mat<XF_2UC1,NROWS,NCOLS,XF_NPPC4> cam_mat_i(NROWS,NCOLS);

	// read input
 	for (int idx = 0 ; idx < (NROWS*NCOLS) ; idx++ ) {
#pragma HLS PIPELINE II=1
		T td = ins.read();
		cam_mat.data[idx] = (uint8_t) td;
	}
	// compute canny
	xf::Canny<FILTER_WIDTH, XF_L1NORM, XF_8UC1, XF_2UC1, NROWS, NCOLS, XF_NPPC1,XF_NPPC4> (cam_mat, cam_mat_i, 20, 54);
	memcpy(outs, cam_mat_i.data, cam_mat_i.size);
}

void cam_edge(hls::stream<uint8_t> &ins,uint64_t *outs)
{
	printf("%s: starting\n",__FUNCTION__);
	uint64_t *t_outs;
#pragma HLS DATAFLOW	
	canny_edge<240,320,uint8_t>(ins,(uint8_t *)t_outs);
	edge_trace<240,320,uint64_t>(t_outs, outs); 
}


// fast corner detection
template <int NROWS, int NCOLS, int NMS, int THRESHOLD>
static void fast_corner (hls::stream<uint8_t> &ins, hls::stream<uint8_t> &outs)
{
	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC1> cam_mat(NROWS,NCOLS);
	xf::Mat<XF_8UC1,NROWS,NCOLS,XF_NPPC1> cam_mat_i(NROWS,NCOLS);
	// read input
 	for (int idx = 0 ; idx < (NROWS*NCOLS) ; idx++ ) {
#pragma HLS PIPELINE II=1
		uint8_t td = ins.read();
		cam_mat.data[idx] = (uint8_t) td;
	}
	xf::fast<NMS,XF_8UC1,NROWS, NCOLS,XF_NPPC1>(cam_mat,cam_mat_i,THRESHOLD);
	int  n_points = 0;
	for (int j = 0; j < cam_mat_i.rows; j++) {
		for (int i = 0; i < (cam_mat_i.cols>>XF_BITSHIFT(XF_NPPC1)); i++) {
#pragma HLS PIPELINE II=1
			unsigned char value = cam_mat_i.data[j*(cam_mat_i.cols>>XF_BITSHIFT(XF_NPPC1))+i];//.at<unsigned char>(j, i);
			if (value != 0) n_points++;
		}
	}
	// cam_mat_i.data[0] = n_points & 0xff;
	// cam_mat_i.data[1] = (n_points >> 8) & 0xff;
	// send output
 	for (int idx = 0 ; idx < (NROWS*NCOLS) ; idx++ ) {
#pragma HLS PIPELINE II=1
		if (idx == 0) outs.write(n_points & 0xff);
		else if (idx == 1) outs.write((n_points >> 8) & 0xff);
		else outs.write(cam_mat_i.data[idx]);
	}
}

void cam_fast_corner(hls::stream<uint8_t> &ins, hls::stream<uint8_t> &outs)
{
	fast_corner<240,320,1,8>(ins, outs);
}

void cam_byte_min_max(hls::stream<uint8_t> &ins,	hls::stream<int> &outs)
{
	calc_min_max<480,640,uint8_t>(ins,outs);
}

void cam_min_max(hls::stream<uint16_t> &ins,	hls::stream<int> &outs)
{
	calc_min_max<480,640>(ins,outs);
}

void flir_min_max(hls::stream<uint16_t> &ins,	hls::stream<int> &outs)
{
	calc_min_max<60,80>(ins,outs);
}

// // MeanShift
#define XF_HEIGHT 480
#define XF_WIDTH  640
#define XF_ROWS	  XF_HEIGHT
#define XF_COLS	  XF_WIDTH
// set the maximum height and width from the objects given for latency report and resource allocation
#define XF_MAX_OBJ_HEIGHT 250
#define XF_MAX_OBJ_WIDTH 250

// maximum number of iterations for centroid convergence
#define XF_MAX_ITERS 4
#define XF_MAX_OBJECTS 4
#define DIFF_ABS(a,b) (a > b ? a - b : b -a)

// Modify the information on objects to be tracked
// coordinate system uses (row, col) where row = 0 and col = 0 correspond to the top-left corner of the input image
const uint16_t X1[XF_MAX_OBJECTS]= {50, 150, 200, 400};        // row coordinates of the top-left corner of all the objects to be tracked
const uint16_t Y1[XF_MAX_OBJECTS]= {50, 150, 200, 400};        // col coordinates of the top-left corner of all the objects to be tracked
const uint16_t HEIGHT_MST[XF_MAX_OBJECTS] = {50, 50, 50, 50}; // height of all the objects to be tracked (measured from top-left corner)
const uint16_t WIDTH_MST[XF_MAX_OBJECTS]  = {50, 50, 50, 50}; // width of all the objects to be tracked (measured from top-left corner)
void MeanShift (hls::stream<uint32_t> &ins, hls::stream<uint16_t> &track_in, hls::stream <uint16_t> &outs)
{
	static uint16_t c_x		[XF_MAX_OBJECTS];
	static uint16_t c_y  		[XF_MAX_OBJECTS];
	static uint16_t h_x 		[XF_MAX_OBJECTS];
	static uint16_t h_y 		[XF_MAX_OBJECTS];
	static  int16_t tlx 		[XF_MAX_OBJECTS];
	static  int16_t tly 		[XF_MAX_OBJECTS];
	static  int16_t brx 		[XF_MAX_OBJECTS];
	static  int16_t bry 		[XF_MAX_OBJECTS];
        static uint16_t track		[XF_MAX_OBJECTS];
	static uint16_t obj_height 	[XF_MAX_OBJECTS];
	static uint16_t obj_width 	[XF_MAX_OBJECTS];
	static uint16_t dx 		[XF_MAX_OBJECTS];
	static uint16_t dy 		[XF_MAX_OBJECTS];
	static int 	track_id      = 0;
	static uint8_t	frame_status  = 0;

	printf("%s: started\n",__FUNCTION__);
	// initialize the first time
	if (!frame_status) {
		for (int i = 0 ; i < XF_MAX_OBJECTS; i++) {
			dx[i] = 0;
			dy[i] = 0;
			h_x[i] = 0;
			h_y[i] = 0;
			c_x[i] = 0;
			c_y[i] = 0;

			obj_height[i] = 0;
			obj_width[i] = 0;

			tlx[i] = 0;
			tly[i] = 0;
			brx[i] = 0;
			bry[i] = 0;
			track[i] = 0;
		}
		track_id = 0;
	} else {
		// something needs to be tracked
		if (!track_in.empty()) {
			int i = track_id;
			uint16_t _tlx, _tly, _brx, _bry;
			_tlx = track_in.read();
			_tly = track_in.read();
			_brx = track_in.read();
			_bry = track_in.read();
			dx[i] = 0;
			dy[i] = 0;
			h_x[i] = (_brx - _tlx)/2;
			h_y[i] = (_bry - _tly)/2;
			c_x[i] = _tlx + h_x[i];
			c_y[i] = _tly + h_y[i];

			obj_height[i] = h_y[i]*2;
			obj_width[i]  = h_x[i]*2;

			tlx[i] = _tlx;
			tly[i] = _tly;
			brx[i] = c_x[i] + h_x[i];
			bry[i] = c_y[i] + h_y[i];
			track[i] = 1;
			track_id++;
			if (track_id == XF_MAX_OBJECTS) track_id = 0;
			frame_status = 0;
		}
	}
	//
	xf::Mat<XF_8UC4, XF_HEIGHT, XF_WIDTH, XF_NPPC1> inMat(XF_HEIGHT,XF_WIDTH);
	// 8 bit 4 channel so 32 bit per pixel
	// copy incoming data into xf::Mat
	for (int i = 0 ; i < (XF_HEIGHT*XF_WIDTH) ; i++) {
#pragma HLS PIPELINE II=1
		inMat.data[i] = ins.read();
	}

	//xf::MeanShift<XF_MAX_OBJECTS,XF_MAX_ITERS,XF_MAX_OBJ_HEIGHT,XF_MAX_OBJ_WIDTH,XF_8UC4,XF_HEIGHT,XF_WIDTH,XF_NPPC1>
	//	(inMat,(uint16_t *)tlx,(uint16_t *)tly,obj_height,obj_width,dx,dy,track,frame_status,XF_MAX_OBJECTS,XF_MAX_OBJECTS,XF_MAX_ITERS);

	for (int k = 0; k < XF_MAX_OBJECTS; k++) {
		// out of range
		if (tlx[k] <= 0 || tly[k] <= 0 || brx[k] >= XF_WIDTH || bry[k] >= XF_HEIGHT)  track[k] = 0;
		if (!track[k]) continue;
		c_x[k] = dx[k];
		c_y[k] = dy[k];
		tlx[k] = c_x[k] - h_x[k];
		tly[k] = c_y[k] - h_y[k];
		brx[k] = c_x[k] + h_x[k];
		bry[k] = c_y[k] + h_y[k];
		dx [k] = dy[k] = 0;
	}

	for (int k = 0; k < XF_MAX_OBJECTS; k++) {
#pragma HLS PIPELINE II=1
		outs.write(track[k]);
		outs.write(tlx[k]);
		outs.write(tly[k]);
		outs.write(brx[k]);
		outs.write(bry[k]);
	}
	frame_status = 1;
	printf("%s: done\n",__FUNCTION__);
}
//
// image_algos.cc ends here
