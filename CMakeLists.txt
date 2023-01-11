# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(fpga-http-server
        VERSION 0.1)

# Target options
set(HTTP_PLATFORM "xilinx_u55c_gen3x16_xdma_3_202210_1" CACHE STRING "Platform string for Vitis.")
set(HTTP_PLATFORM_PART "xcu55c-fsvh2892-2L-e" CACHE STRING "LALAL")
set(HTTP_IP_NAME "http_top" CACHE STRING "HTTP IP top module")
set(HTTP_KERNEL_NAME "http" CACHE STRING "HTTP Kernel name")
option(HTTP_ENABLE_PROFILING "Collect profiling information." OFF)
option(HTTP_ENABLE_DEBUGGING "Inject debugging cores to design." OFF)

# Domain options
set(HTTP_LISTEN_PORT "80" CACHE STRING "Listen port number for the HTTP Server.")

# Source
set(HTTP_HW_HLS_SOURCE_FILES
  "hw/hls/headline_parser.cc"
  "hw/hls/http.cc"
  "hw/hls/listen_port.cc"
  "hw/hls/method_parser.cc"
  "hw/hls/req_payload_parser.cc"
  "hw/hls/request_processor.cc"
  "hw/hls/response_aligner.cc"
  "hw/hls/response_processor.cc"
  "hw/hls/status_code_parser.cc")

set(HTTP_HW_HLS_SOURCE_FILES_H
  "hw/hls/headline_parser.h"
  "hw/hls/http_lib.h"
  "hw/hls/http.h"
  "hw/hls/listen_port.h"
  "hw/hls/method_parser.h"
  "hw/hls/req_payload_parser.h"
  "hw/hls/request_processor.h"
  "hw/hls/response_aligner.h"
  "hw/hls/response_processor.h"
  "hw/hls/status_code_parser.h"
  "hw/hls/tcp_utils.h")

SET(HTTP_HW_RTL_SOURCE_FILES
  "hw/rtl/http_control_s_axi.sv"
  "hw/rtl/http_core.sv"
  "hw/rtl/http.sv")

set(HTTP_SW_SOURCE_FILES
  "sw/host.cc")

#
# vitis-network-stack
#

set(HTTP_VNS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/vitis-network-stack)
set(FDEV_NAME "u55c" CACHE STRING "FPGA Device Family")
set(TCP_STACK_EN 1 CACHE BOOL "Enable TCP/IP stack")
set(UDP_STACK_EN 0 CACHE BOOL "Enable UDP/IP stack")

add_subdirectory(vitis-network-stack)

#
# http hls ip
#

add_vitis_ip(${HTTP_IP_NAME}
            FILES ${HTTP_HW_HLS_SOURCE_FILES} ${HTTP_HW_HLS_SOURCE_FILES_H}
            TB_FILES
              "hw/hls/http.spec.cc"
            PLATFORM_PART ${HTTP_PLATFORM_PART}
            VENDOR "ethz.systems.fpga"
            DISPLAY_NAME "FPGA HTTP Server Stack"
            DESCRIPTION "Middleware between Application HTTP Request-Response and TCP-IP stack."
            VERSION ${PROJECT_VERSION})

#
# http rtl kernel
#

add_vivado_kernel(${HTTP_KERNEL_NAME}
                  PACKAGE_TCL_PATH ${CMAKE_CURRENT_BINARY_DIR}/hw/vitis/package.tcl
                  FILES ${HTTP_HW_RTL_SOURCE_FILES}
                  IPS ${HTTP_IP_NAME}
                  XML_PATH hw/vitis/kernel.xml)

#
# static-pages application
#

add_subdirectory(examples/static_pages)

#
# hw_emu mock
#

add_subdirectory(hw/mock)

#
# program bitstream
#

set(PROGRAM_BITSTREAM "${CMAKE_BINARY_DIR}/artifacts/http.xclbin")
set(PROGRAM_VPP_FLAGS "--save-temps")
set(PROGRAM_VPP_LDFLAGS "")
set(PROGRAM_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/http.link.xclbin
    COMMENT "Linking bitstream .xclbin"
    DEPENDS ${CMAKE_BINARY_DIR}/http.xo ${CMAKE_BINARY_DIR}/cmac_krnl.xo ${CMAKE_BINARY_DIR}/network_krnl.xo ${CMAKE_BINARY_DIR}/static_pages.xo
    COMMAND ${Vitis_COMPILER} ${PROGRAM_VPP_FLAGS} -l ${PROGRAM_VPP_LDFLAGS} --temp_dir ${PROGRAM_TEMP_DIR}
            -o ${PROGRAM_TEMP_DIR}/http.link.xclbin
            ${CMAKE_BINARY_DIR}/http.xo ${CMAKE_BINARY_DIR}/cmac_krnl.xo ${CMAKE_BINARY_DIR}/network_krnl.xo ${CMAKE_BINARY_DIR}/static_pages.xo
            --advanced.param compiler.userPostDebugProfileOverlayTcl=${CMAKE_CURRENT_BINARY_DIR}/vitis-network-stack/scripts/post_sys_link.tcl
            --config ${CMAKE_CURRENT_SOURCE_DIR}/hw/vitis/connectivity.cfg --vivado.synth.jobs 20 --vivado.impl.jobs 20
            --platform ${HTTP_PLATFORM})

add_custom_command(OUTPUT ${PROGRAM_BITSTREAM}
  COMMENT "Building bitstream .xclbin"
  COMMAND ${Vitis_COMPILER} -p ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/http.link.xclbin -t hw --platform ${HTTP_PLATFORM}
          -o ${PROGRAM_BITSTREAM}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/http.link.xclbin)

add_custom_target(bitstream
  DEPENDS ${PROGRAM_BITSTREAM})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hw/vitis/package.tcl.in ./hw/vitis/package.tcl)