cmake_minimum_required(VERSION 2.6)

project(vsi_hls_examples)

set(TARGET_EXAMPLE_DIR ${TARGET_DIR}/hls_examples)

install(FILES
	sort/sort.cc
	led_blinker/blink.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/sort)

install(FILES
	led_blinker/blink.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/led_blinker)
