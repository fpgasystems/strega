# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(hybrid_static_pages)

# Target options
set(BENCH_PLATFORM "xilinx_u55c_gen3x16_xdma_3_202210_1" CACHE STRING "Platform string for Vitis.")

# #
# # hlslib
# #

# set(BENCH_HLSLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../hlslib)
# include_directories(SYSTEM ${BENCH_HLSLIB_DIR}/include ${Vitis_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../xilinx-cmake)
find_package(Vitis REQUIRED)

set(XIT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../vitis-network-stack/common/includes)
include_directories(SYSTEM ${Vitis_INCLUDE_DIRS} ${XIT_INCLUDE_DIR})


add_vitis_kernel(benchmark_krnl
                  FILES "kernel.cc"
                  KERNEL "hybrid_static_pages")

add_vitis_program(benchmark_pgm
                  ${BENCH_PLATFORM}
                  KERNELS benchmark_krnl
                  SAVE_TEMPS ON  # Forwards --save-temps to Vitis
                  DEBUGGING OFF  # Enables Chipscope debugging on all interfaces
                  PROFILING OFF) # Enables profiling for stalls, data transfers, and execution

# Host code to launch kernel
add_executable(host.exe host.cc ${XIT_INCLUDE_DIR}/xcl2/xcl2.cpp)
target_link_libraries(host.exe ${Vitis_LIBRARIES})