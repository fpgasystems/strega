# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(health_check
        VERSION 0.1)

# Options

#
# health_check
#

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