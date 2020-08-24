cmake_minimum_required (VERSION 2.6)

if(NOT DEFINED ENV{XILINX_VITIS})
	message( FATAL_ERROR "Environment variable XILINX_VITIS not set. Must source Vitis settings." )
endif()
include(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER   $ENV{XILINX_VITIS}/cardano/tps/lnx64/gcc/bin/gcc CACHE STRING "c compiler")
set(CMAKE_CXX_COMPILER $ENV{XILINX_VITIS}/cardano/tps/lnx64/gcc/bin/g++ CACHE STRING "c++ compiler")
set(CMAKE_AR  $ENV{XILINX_VITIS}/cardano/tps/lnx64/gcc/bin/gcc-ar CACHE STRING "GCC ar")

project (zynq_ps)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(VSI_TARGET_SDK $ENV{VSI_INSTALL}/target/X86)
set(CMAKE_PREFIX_PATH ${VSI_TARGET_SDK})
string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}.x86_64" HOST_VAR)
set(VSI_HOST_DIR $ENV{VSI_INSTALL}/host/${HOST_VAR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(VITIS_X86CMPLR $ENV{XILINX_VITIS}/cardano/tps/lnx64/gcc)
set(COMPILE_FLAGS "${COMPILE_FLAGS} --sysroot=${VITIS_X86CMPLR} -isysroot / -Wl,--dynamic-linker=${VITIS_X86CMPLR}/lib/ld-linux-x86-64.so.2 -Wl,-rpath=${VITIS_X86CMPLR}/lib/:${VITIS_X86CMPLR}/lib64/:${VITIS_X86CMPLR}/lib/gcc/x86_64-pc-linux-gnu/6.2.0/ ")

set(CMAKE_C_FLAGS "${COMPILE_FLAGS} ${CMAKE_C_FLAGS} -std=c11 -g -DDEBUG -O0 ")
set(CMAKE_CXX_FLAGS "${COMPILE_FLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 -g -DDEBUG -O0 -fpermissive ")


include_directories(
	$ENV{PROJECT_PATH}/vsi_auto_gen/sw/system_1/zynq_ps/include
	$ENV{PROJECT_PATH}/vsi_auto_gen/sw/system_1/zynq_ps/public
	$ENV{VSI_INSTALL}/target/common/include
	$ENV{XILINX_DIR}/Vivado/2020.1/lnx64/tools/fpo_v7_0/
	$ENV{XILINX_DIR}/Vivado/2020.1/include
	/tools/Xilinx/Vivado/2020.1/lnx64/tools/fpo_v7_0/
	/tools/Xilinx/Vivado/2020.1/include
)

link_directories(
	${VSI_TARGET_SDK}/lib
	$ENV{PROJECT_PATH}/vsi_auto_gen/sw/system_1/build/zynq_ps
)



add_executable(main_sc
	main_sc.cxx
)

target_link_libraries(main_sc
	zynq_ps
)
