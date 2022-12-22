# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(fpga-http-server
        VERSION 0.1)

# Target options
#set(HTTP_PLATFORM "xilinx_u55c_gen3x16_xdma_3_202210_1" CACHE STRING "Platform string for Vitis.")
set(HTTP_PLATFORM_PART "xcu55c-fsvh2892-2L-e" CACHE STRING "LALAL")
option(HTTP_ENABLE_PROFILING "Collect profiling information." OFF)
option(HTTP_ENABLE_DEBUGGING "Inject debugging cores to design." OFF)

# Domain options
set(HTTP_LISTEN_PORT "80" CACHE STRING "Listen port number for the HTTP Server.")

# Infra
set(HTTP_IP_DIR ${CMAKE_BINARY_DIR}/ips CACHE STRING "IP build directory")

# Source
set(HTTP_HW_SOURCE_FILES
  "hw/hls/headline_parser.cc"
  "hw/hls/http.cc"
  "hw/hls/listen_port.cc"
  "hw/hls/method_parser.cc"
  "hw/hls/req_payload_parser.cc"
  "hw/hls/request_processor.cc"
  "hw/hls/response_processor.cc"
  "hw/hls/status_code_parser.cc")

set(HTTP_HW_SOURCE_FILES_H
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

add_subdirectory(vitis-network-stack)

#
# fpga-http-server ip
#

add_vitis_ip(http_top #${PROJECT_NAME}
            FILES ${HTTP_HW_SOURCE_FILES} ${HTTP_HW_SOURCE_FILES_H}
            TB_FILES
              "hw/http.spec.cc"
            PLATFORM_PART ${HTTP_PLATFORM_PART}
            VENDOR "ethz.systems.fpga"
            DISPLAY_NAME "FPGA HTTP Server Stack"
            DESCRIPTION "Middleware between Application HTTP Request-Response and TCP-IP stack."
            VERSION ${PROJECT_VERSION}
            IP_DIR ${HTTP_IP_DIR})

#
# static-pages application
#

add_subdirectory(examples/static_pages)

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
#                  FILES ${HTTP_HW_SOURCE_FILES}
#                  KERNEL "http_krnl"
#                  DEPENDS ${HTTP_HW_SOURCE_FILES_H}
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
