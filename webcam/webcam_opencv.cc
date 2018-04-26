// webcam_opencv.cc ---
//
// Filename: webcam_opencv.cc
// Description:
// Author: Matt
// Maintainer: System View Inc
// Created: 4/26/2018
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
//  This file contains functions for opening a media camera
//  and reading / writing to fpga

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


#ifndef __VSI_HLS_SYN__


 #include "opencv2/opencv.hpp"
 #include "opencv2/videoio.hpp"
 #include <hls_stream.h>
 #include <unistd.h>
 using namespace cv;

//Displays webcam image on arm without any fpga processing
 void display_sw_only()
 {
     VideoCapture cap(0); // open the default camera
     if(!cap.isOpened()){  // check if we succeeded
 		printf("open failed!\n");
 		exit(0);
 	}

     Mat edges;
     namedWindow("edges",1);

     for(;;)
     {
         Mat frame;
         cap >> frame; // get a new frame from camera
         cvtColor(frame, edges, COLOR_BGR2GRAY);
		 GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
         Canny(edges, edges, 0, 30, 3);

         imshow("edges", edges);

         if(waitKey(30) >= 0) break;
     }
     // the camera will be deinitialized automatically in VideoCapture destructor
     exit(0);
 }

 //Displays webcam image on arm with fpga supplying min and max values

void display_min_max(hls::stream<uint16_t> &out_image,	hls::stream<int> &in_points)
{

	unsigned int wc_minmax[6];

    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()){  // check if we succeeded
		printf("open failed!\n");
		exit(0);
	}

    Mat edges;
    namedWindow("edges",1);

    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);

		if (edges.isContinuous()) {
			out_image.write(edges.data,edges.total()*edges.elemSize());
		} else {
			printf("Cannot handle non-contiguos image\n");
		}

		for (int i = 0 ; i < 6 ; i++) {
			wc_minmax[i] = in_points.read();
		}

		printf("Min Value: 0x%x at (0x%x, 0x%x)\n",  wc_minmax[0], wc_minmax[2], wc_minmax[4] );
		printf("Max Value: 0x%x at (0x%x, 0x%x)\n",  wc_minmax[1], wc_minmax[3], wc_minmax[5] );

		cv::drawMarker(edges, cv::Point(wc_minmax[2],wc_minmax[4]), cv::Scalar(255,255,255), MARKER_CROSS, 10, 1);				//min
		cv::drawMarker(edges, cv::Point(wc_minmax[3],wc_minmax[5]), cv::Scalar(0,0,0), MARKER_STAR, 10, 1);				//max

        imshow("edges", edges);

        if(waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    exit(0);
}

//Displays webcam image on arm after image has been processed by the FPGA


void display_fpga(hls::stream<uint8_t> &out_image,	hls::stream<uint8_t> &in_image)
{


    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()){  // check if we succeeded
		printf("open failed!\n");
		exit(0);
	}

    Mat edges;
	Mat frame;
	Mat small;
	Mat recieve_image;


    namedWindow("edges",1);

    for(;;)
    {
        cap >> frame; // get a new frame from camera

		//resize smaller due to fpga limitations
		resize(frame,small,Size(320,240));

        cvtColor(small, edges, COLOR_BGR2GRAY);

		if (edges.isContinuous()) {
			printf("Wiriting %d(total) * %d(size) 0x%x\n",edges.total(),edges.elemSize(), edges.total()*edges.elemSize());
			out_image.write(edges.data,edges.total()*edges.elemSize());
		} else {
			printf("Cannot handle non-contiguos image\n");
		}
		printf("Reading %d(total) * %d(size) 0x%x\n",edges.total(),edges.elemSize(), edges.total()*edges.elemSize());

		recieve_image = Mat::zeros(Size(320,240), CV_8UC1);

		for(int i = 0; i < 320; i++)
		{
			printf("Read %d\n",i);
			//in_image.read(recieve_image.data[x],edges.total()*edges.elemSize());
			in_image.read(recieve_image.data[i*240],240);
		}


		resize(recieve_image,frame,Size(640,480));

        imshow("edges", frame);

        if(waitKey(10000) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    exit(0);
}

#endif

//
// webcam_opencv.cc ends here
