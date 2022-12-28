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

# Infra
set(HTTP_IP_DIR ${CMAKE_BINARY_DIR}/ips CACHE STRING "IP build directory")
set(HTTP_KERNEL_DIR ${CMAKE_BINARY_DIR}/kernels CACHE STRING "Kernel build directory")
set(HTTP_BUILD_DIR "http_kernel_build" CACHE STRING "HTTP building directory")

# Source
set(HTTP_HW_HLS_SOURCE_FILES
  "hw/hls/headline_parser.cc"
  "hw/hls/http.cc"
  "hw/hls/listen_port.cc"
  "hw/hls/method_parser.cc"
  "hw/hls/req_payload_parser.cc"
  "hw/hls/request_processor.cc"
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
  "hw/hls/response_processor.h"
  "hw/hls/status_code_parser.h"
  "hw/hls/tcp_utils.h")

SET(HTTP_HW_RTL_SOURCE_FILES
  "hw/rtl/http_control_s_axi.sv"
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
set(IPREPO_DIR ${HTTP_IP_DIR} CACHE STRING "IP build directory")
set(VNS_KERNEL_DIR ${HTTP_KERNEL_DIR} CACHE STRING "Kernel build directory")

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
            VERSION ${PROJECT_VERSION}
            IP_DIR ${HTTP_IP_DIR})

#
# http rtl kernel
#

set(HTTP_IP_CMD "ip")
set(HTTP_KERNEL_CMD "kernel")

set(GENXO_XO_PATH ${HTTP_KERNEL_DIR}/${HTTP_KERNEL_NAME}.xo)
set(GENXO_KRNL_NAME ${HTTP_KERNEL_NAME})
set(GENXO_XML_PATH ${CMAKE_CURRENT_SOURCE_DIR}/hw/vitis/kernel.xml)
set(GENXO_TCL_PATH ./hw/vitis/package.tcl)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vitis-network-stack/scripts/gen_xo.tcl.in ./hw/vitis/gen_xo.tcl)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hw/vitis/package.tcl.in ./hw/vitis/package.tcl)

add_custom_target(${HTTP_KERNEL_CMD}.${HTTP_KERNEL_NAME}
  COMMENT "Exporting kernel ${HTTP_KERNEL_NAME} to .xo"
  COMMAND ${Vitis_VIVADO} -mode batch -source ./hw/vitis/gen_xo.tcl
  DEPENDS ${HTTP_HW_RTL_SOURCE_FILES} ${HTTP_IP_CMD}.${HTTP_IP_NAME})

add_dependencies(${HTTP_KERNEL_CMD} ${HTTP_KERNEL_CMD}.${HTTP_KERNEL_NAME})

#
# static-pages application
#

add_subdirectory(examples/static_pages)

#
# program bitstream
#

set(PROGRAM_BITSTREAM "${CMAKE_BINARY_DIR}/artifacts/http.xclbin")
set(PROGRAM_VPP_FLAGS "--save-temps")
set(PROGRAM_VPP_LDFLAGS "")
set(PROGRAM_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/http.link.xclbin
    COMMENT "Linking bitstream .xclbin"
    COMMAND ${Vitis_COMPILER} ${PROGRAM_VPP_FLAGS} -l ${PROGRAM_VPP_LDFLAGS} --temp_dir ${PROGRAM_TEMP_DIR}
            -o ${PROGRAM_TEMP_DIR}/http.link.xclbin
            ${HTTP_KERNEL_DIR}/http.xo ${HTTP_KERNEL_DIR}/cmac_krnl.xo ${HTTP_KERNEL_DIR}/network_krnl.xo ${HTTP_KERNEL_DIR}/static_pages.xo
            --advanced.param compiler.userPostDebugProfileOverlayTcl=${CMAKE_CURRENT_BINARY_DIR}/vitis-network-stack/scripts/post_sys_link.tcl
            --config ${CMAKE_CURRENT_SOURCE_DIR}/hw/vitis/connectivity.cfg
            --platform ${HTTP_PLATFORM})

add_custom_command(OUTPUT ${PROGRAM_BITSTREAM}
  COMMENT "Building bitstream .xclbin"
  COMMAND ${Vitis_COMPILER} -p ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/http.link.xclbin -t hw --platform ${HTTP_PLATFORM}
          -o ${PROGRAM_BITSTREAM}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/http.link.xclbin)

add_custom_target(bitstream
  DEPENDS ${PROGRAM_BITSTREAM})
#
# hlslib
#

# set(HTTP_HLSLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/hlslib)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HTTP_HLSLIB_DIR}/cmake)
# include_directories(SYSTEM ${HTTP_HLSLIB_DIR}/include)

# find_package(Vitis REQUIRED)
# include_directories(SYSTEM ${Vitis_INCLUDE_DIRS})

# add_vitis_kernel(network
#                  FILES hw/hls/headline_parser.cc hw/hls/http.cc hw/hls/listen_port.cc
#                  hw/hls/method_parser.cc hw/hls/req_payload_parser.cc hw/hls/request_processor.cc
#                  KERNEL "network_krnl")
#                  #DEPENDS include/MyHeader.h include/OtherDependency.h
#                  #INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include hlslib/include
#                  #PORT_MAPPING "ddr0:DDR[0]" "ddr1:DDR[1]")

# add_vitis_kernel(http
#                  FILES ${HTTP_HW_HLS_SOURCE_FILES}
#                  KERNEL "http_krnl"
#                  DEPENDS ${HTTP_HW_HLS_SOURCE_FILES_H}
#                  HLS_FLAGS "-DHTTP_LISTEN_PORT=" ${HTTP_LISTEN_PORT})
#                  #INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include hlslib/include
#                  #PORT_MAPPING "ddr0:DDR[0]" "ddr1:DDR[1]")

# add_vitis_program(MyProgram
#                  ${HTTP_PLATFORM}
#                  KERNELS "cmac_krml" "network_krnl" "http_krnl"
#                  CONNECTIVITY
#                     "network_krnl_1.m_axis_udp_rx:http_krnl_1.s_axis_udp_rx"
#                     "network_krnl_1.m_axis_udp_rx_meta:http_krnl_1.s_axis_udp_rx_meta"
#                     "network_krnl_1.m_axis_tcp_open_status:http_krnl_1.s_axis_tcp_open_status"
#                     "network_krnl_1.m_axis_tcp_port_status:http_krnl_1.s_axis_tcp_port_status"
#                     "network_krnl_1.m_axis_tcp_notification:http_krnl_1.s_axis_tcp_notification"
#                     "network_krnl_1.m_axis_tcp_rx_meta:http_krnl_1.s_axis_tcp_rx_meta"
#                     "network_krnl_1.m_axis_tcp_rx_data:http_krnl_1.s_axis_tcp_rx_data"
#                     "network_krnl_1.m_axis_tcp_tx_status:http_krnl_1.s_axis_tcp_tx_status"
#                     "http_krnl_1.m_axis_udp_tx:network_krnl_1.s_axis_udp_tx"
#                     "http_krnl_1.m_axis_udp_tx_meta:network_krnl_1.s_axis_udp_tx_meta"
#                     "http_krnl_1.m_axis_tcp_listen_port:network_krnl_1.s_axis_tcp_listen_port"
#                     "http_krnl_1.m_axis_tcp_open_connection:network_krnl_1.s_axis_tcp_open_connection"
#                     "http_krnl_1.m_axis_tcp_close_connection:network_krnl_1.s_axis_tcp_close_connection"
#                     "http_krnl_1.m_axis_tcp_read_pkg:network_krnl_1.s_axis_tcp_read_pkg"
#                     "http_krnl_1.m_axis_tcp_tx_meta:network_krnl_1.s_axis_tcp_tx_meta"
#                     "http_krnl_1.m_axis_tcp_tx_data:network_krnl_1.s_axis_tcp_tx_data"
#                     "cmac_krnl_1.axis_net_rx:network_krnl_1.axis_net_rx"
#                     "network_krnl_1.axis_net_tx:cmac_krnl_1.axis_net_tx"
#                  CONFIG scripts/my_config.cfg  # Given as --config to Vitis
#                  SAVE_TEMPS ON  # Forwards --save-temps to Vitis
#                  BUILD_FLAGS "-Os --export_script"
#                  DEBUGGING OFF  # Enables Chipscope debugging on all interfaces
#                  PROFILING ON)  # Enables profiling for stalls, data transfers, and execution
