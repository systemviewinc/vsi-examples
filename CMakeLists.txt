cmake_minimum_required(VERSION 2.6)

project(vsi_hls_examples)

set(TARGET_EXAMPLE_DIR ${STAGING_DIR}/target/common/hls_examples)

install(DIRECTORY
	aie_example_kernels
	aie_vectorize_tests
	benchmark_memory
	benchmark_memory_simulation
	blocking_read
	device
	example_1
	facial_landmark_detection
	fft
	flir_sw
	hls_master
	hls_video
	image_algos
	include
	led_blinker
	lab
	lod_image_algos
	memory
	memoryrwtest
	microphone
	pass_thru_r5
	preset_yaml_files
	rdma_helper
	regex_packet
	regression
	sort
	sort_x86
	stream_ethernet
	stream_mux
	univariate_linear_regression
	webcam
	webcam_opencv
	xilinx_vlib
	aie_emulation
	streamdecoder
	simulator_example
	rf_demo
	stream_and_mem_test
DESTINATION ${TARGET_EXAMPLE_DIR})
