cmake_minimum_required(VERSION 2.6)

project(vsi_hls_examples)

set(TARGET_EXAMPLE_DIR ${STAGING_DIR}/target/common/hls_examples)

install(DIRECTORY
	example_1
	sort
	led_blinker
	memory
	regex_packet
	stream_mux
	blocking_read
	device
	webcam
	webcam_opencv
	include
	microphone
	memoryrwtest
	flir_sw
	xilinx_vlib
	image_algos
	pass_thru_r5
	facial_landmark_detection
	univariate_linear_regression
	hls_video
	hls_master
	regression
DESTINATION ${TARGET_EXAMPLE_DIR})
