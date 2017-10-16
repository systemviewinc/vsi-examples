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
#include "ap_int.h"
#define CLEAR(x) memset(&(x), 0, sizeof(x))
struct buffer {
	void   *start;
	size_t length;
};
#define WC_NROWS 480
#define WC_NCOLS 640
#define WC_IMGSIZE (3*WC_NROWS*WC_NCOLS)
#define WC_BPP	 3

struct wc_vs {
	ap_uint<8> data;
	ap_uint<1>  last;
};
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
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
extern "C" {
#include "libv4l2.h"
#include "libv4lconvert.h"
}

#include "double_buffer.h"
typedef struct { unsigned char buff[WC_IMGSIZE+1];} wc_db_t;

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
	char                           *dev_name;
	struct v4lconvert_data 	      *v4lconvert_data;
	struct v4l2_format 	       src_fmt;
	unsigned char 		      *dst_buf;
	struct buffer                  *buffers;
 public:
	webcam();
	~webcam();
	void webcam_capture_image();
	ProducerConsumerDoubleBuffer<wc_db_t> wc_db;
	static void xioctl(int fh, int request, void *arg);
};
#else
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#endif

#endif

/* webcam.h ends here */
