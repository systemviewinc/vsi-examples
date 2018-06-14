cmake_minimum_required (VERSION 2.6)

project (zynq_ps)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(VSI_TARGET_SDK $ENV{VSI_INSTALL}/target/X86)
set(CMAKE_PREFIX_PATH ${VSI_TARGET_SDK})
string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}.x86_64" HOST_VAR)
set(VSI_HOST_DIR $ENV{VSI_INSTALL}/host/${HOST_VAR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${COMPILE_FLAGS} ${CMAKE_C_FLAGS} -std=c11 -g -DDEBUG -O0 ")
set(CMAKE_CXX_FLAGS "${COMPILE_FLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 -g -DDEBUG -O0 -fpermissive ")


include_directories(
	$ENV{PROJECT_PATH}/vsi_auto_gen/sw/system_1/zynq_ps/include
	$ENV{PROJECT_PATH}/vsi_auto_gen/sw/system_1/zynq_ps/public
	$ENV{VSI_INSTALL}/target/common/include
	$ENV{XILINX_DIR}/Vivado/2018.1/lnx64/tools/fpo_v7_0/
	$ENV{XILINX_DIR}/Vivado/2018.1/include
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

