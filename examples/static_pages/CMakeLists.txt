# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(static_pages
        VERSION 0.1)

# Options

#
# static_pages
#

set(APP_HW_HLS_SOURCE_FILES
  "static_pages.cc")

set(APP_HW_HLS_SOURCE_FILES_H
  "static_pages.h")

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/static_pages.xo
  COMMENT "Exporting kernel .xo static_pages"
  COMMAND ${Vitis_COMPILER} -c -k static_pages
          -o ${CMAKE_BINARY_DIR}/static_pages.xo
          --input_files ${CMAKE_CURRENT_SOURCE_DIR}/${APP_HW_HLS_SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/${APP_HW_HLS_SOURCE_FILES_H}
          --platform ${HTTP_PLATFORM}
  DEPENDS ${APP_HW_HLS_SOURCE_FILES} ${APP_HW_HLS_SOURCE_FILES_H})

add_custom_target(kernel.static_pages DEPENDS ${CMAKE_BINARY_DIR}/static_pages.xo)
