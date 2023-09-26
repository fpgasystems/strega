# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(uppercase
        VERSION 0.1)

# Options
set(HTTP_PLATFORM "xilinx_u55c_gen3x16_xdma_3_202210_1" CACHE STRING "Platform string for Vitis.")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../../xilinx-cmake)
find_package(Vitis REQUIRED)

#
# uppercase
#

add_executable(MyHostExecutable host.cc)
include_directories(${Vitis_INCLUDE_DIRS})
target_link_libraries(MyHostExecutable ${Vitis_LIBRARIES})

set(APP_HW_HLS_SOURCE_FILES
  "kernel.cc")

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xo
  COMMENT "Exporting kernel .xo ${PROJECT_NAME}"
  COMMAND ${Vitis_COMPILER} -c -k ${PROJECT_NAME}
          -o ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xo
          --input_files ${CMAKE_CURRENT_SOURCE_DIR}/${APP_HW_HLS_SOURCE_FILES}
          --platform ${HTTP_PLATFORM}
  DEPENDS ${APP_HW_HLS_SOURCE_FILES})

add_custom_target(kernel.${PROJECT_NAME} DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xo)

set(PROGRAM_BITSTREAM "${CMAKE_BINARY_DIR}/benchmarks/${PROJECT_NAME}.xclbin")
set(PROGRAM_VPP_FLAGS "--save-temps")
set(PROGRAM_VPP_LDFLAGS "")
set(PROGRAM_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/${PROJECT_NAME}.link.xclbin
    COMMENT "Linking bitstream .xclbin"
    DEPENDS kernel.${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xo
    COMMAND ${Vitis_COMPILER} ${PROGRAM_VPP_FLAGS} -l ${PROGRAM_VPP_LDFLAGS} --temp_dir ${PROGRAM_TEMP_DIR}
            -o ${PROGRAM_TEMP_DIR}/${PROJECT_NAME}.link.xclbin
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xo
            --platform ${HTTP_PLATFORM})

add_custom_command(OUTPUT ${PROGRAM_BITSTREAM}
  COMMENT "Building bitstream .xclbin"
  COMMAND ${Vitis_COMPILER} -p ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/${PROJECT_NAME}.link.xclbin -t hw --platform ${HTTP_PLATFORM}
          -o ${PROGRAM_BITSTREAM}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tmp_xclbin/${PROJECT_NAME}.link.xclbin)

add_custom_target(bitstream
  DEPENDS ${PROGRAM_BITSTREAM})