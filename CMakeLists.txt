cmake_minimum_required(VERSION 2.6)

project(vsi_hls_examples)

set(TARGET_EXAMPLE_DIR ${STAGING_DIR}/target/common/hls_examples)

install(FILES
	sort/sort.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/sort)

install(FILES
	led_blinker/blink.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/led_blinker)

install(FILES
	memory/memory.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/memory)

install(FILES
	regex_packet/compile/regex.h
	DESTINATION ${TARGET_EXAMPLE_DIR}/regex_packet/compile)

install(FILES
	regex_packet/compile/regex_comp.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/regex_packet/compile)

install(FILES
	regex_packet/match/regex.h
	DESTINATION ${TARGET_EXAMPLE_DIR}/regex_packet/match)

install(FILES
	regex_packet/match/regex_exec.cc
	DESTINATION ${TARGET_EXAMPLE_DIR}/regex_packet/match)
