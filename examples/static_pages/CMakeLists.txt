# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(static_pages
        VERSION 0.1)

# Options

#
# static_pages
#

# add_vitis_ip(${PROJECT_NAME}
#             FILES "hw/static_pages.cc" "hw/static_pages.h"
#             PLATFORM_PART ${HTTP_PLATFORM_PART}
#             VENDOR "ethz.systems.fpga"
#             DISPLAY_NAME "FPGA Static Page WebServer"
#             DESCRIPTION "Server static resources over http"
#             VERSION ${PROJECT_VERSION}
#             IP_DIR ${HTTP_IP_DIR})