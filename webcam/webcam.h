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

class webcam {
 private:
	struct v4l2_format              fmt;
	struct v4l2_buffer              buf;
	struct v4l2_requestbuffers      req;
	enum v4l2_buf_type                     type;
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
};
#else
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#endif

#endif
