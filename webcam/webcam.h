/* webcam.h ---
 *
 * Filename: webcam.h
 * Description:
 * Author: Sandeep
 * Maintainer: System View Inc
 * Created: Thu Oct 12 13:44:41 2017 (-0700)
 * Version:
 * Package-Requires: ()
 * Last-Updated:
 *           By:
 *     Update #: 0
 * URL:
 * Doc URL:
 * Keywords:
 * Compatibility:
 *
 */

/* Commentary:
 *
 *
 *
 */

/* Change Log:
 *
 *
 */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Code: */


#ifndef WEBCAM_H
#define WEBCAM_H
#include <hls_stream.h>
#include <vsi_device.h>
#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
	void   *start;
	size_t length;
};
#define WC_NROWS 480
#define WC_NCOLS 640
#define WC_BPP	 3
#define WC_IMGSIZE (WC_BPP*WC_NROWS*WC_NCOLS)
#define WC_IMGSIZE_BW (WC_NROWS*WC_NCOLS)
#define WC_HIMGSIZE_BW ((WC_NROWS/2)*(WC_NCOLS)/2)
/* ****************************************************************
 * 			non sythesizable portion
 * **************************************************************** */

#ifndef __VSI_HLS_SYN__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <atomic>
#include <mutex>
extern "C" {
#include "libv4l2.h"
#include "libv4lconvert.h"
}

#include "double_buffer.h"

/* ****************************************************************
 *  The Webcam class
 * **************************************************************** */

class webcam {
 private:
	struct v4l2_format              fmt;
	struct v4l2_buffer              buf;
	struct v4l2_requestbuffers      req;
	enum v4l2_buf_type              type;
	fd_set                          fds;
	struct timeval                  tv;
	int                             r, fd;
	unsigned int                    n_buffers;
	struct v4lconvert_data 	      *v4lconvert_data;
	struct v4l2_format 	       src_fmt;
	unsigned char 		      *dst_buf;
	struct buffer                  *buffers;
 public:
	std::atomic<bool>	       	paused;
	std::atomic<bool>		running;
	std::mutex			o_lock;  // lock for overlay image 
	cv::Mat 			o_image; // image to overlay for display
	double_buffer<cv::Mat> wc_db; // input
	double_buffer<cv::Mat> wc_ddb;// display
	webcam(const char *);
	~webcam();
	void webcam_capture_image();
	void webcam_display_image();
	int xioctl(int fh, int request, void *arg);
	virtual void webcam_cvt_process_image(hls::stream<uint32_t> &, hls::stream<int> &, hls::stream<int>&,
					      std::function<void (cv::Mat &, cv::Mat &)> const &cvt =
					      [] (cv::Mat &in, cv::Mat &out) {
						      out = in.clone();
					      });
	virtual void webcam_cvt_process_image(uint32_t oa[WC_IMGSIZE], hls::stream<int> &, hls::stream<int>&,
					      std::function<void (cv::Mat &, cv::Mat &)> const &cvt =
					      [] (cv::Mat &in, cv::Mat &out) {
						      out = in.clone();
					      });

	virtual void webcam_cvt_process_image(vsi::device &mem, hls::stream<int>&,
					      std::function<void (cv::Mat &, cv::Mat &)> const &cvt =
					      [] (cv::Mat &in, cv::Mat &out) {
						      out = in.clone();
					      });
};

class opencv_display {
 public:
	double_buffer<cv::Mat> wc_db[2];
	char window_name[256];
	opencv_display(const char *wn) {
		strcpy(window_name,wn);
	};
	void opencv_display_image(hls::stream<int> &);
};
#include "ap_int.h"
typedef struct {
	ap_uint<32>  	   data;
	ap_uint<1> last;
} st;


#else
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#endif

#endif

/* webcam.h ends here */
