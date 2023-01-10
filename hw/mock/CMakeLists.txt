# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

#
# http kernel
#

set(MOCK_HTTP_XO ${CMAKE_CURRENT_BINARY_DIR}/mock/http.hw_emu.xo)
set(MOCK_HTTP_SOURCES
  "hw/hls/headline_parser.cc"
  "hw/hls/http.cc"
  "hw/hls/listen_port.cc"
  "hw/hls/method_parser.cc"
  "hw/hls/req_payload_parser.cc"
  "hw/hls/request_processor.cc"
  "hw/hls/response_processor.cc"
  "hw/hls/status_code_parser.cc")
hlslib_make_paths_absolute(MOCK_HTTP_SOURCES ${MOCK_HTTP_SOURCES})
add_custom_command(OUTPUT ${MOCK_HTTP_XO}
  COMMENT "Compiling mock kernel http.hw_emu.xo"
  DEPENDS ${MOCK_HTTP_SOURCES}
  COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -c -o ${MOCK_HTTP_XO} -k http_top --input_files ${MOCK_HTTP_SOURCES})

#
# tcp_mock kernel
#

set(MOCK_TCP_XO ${CMAKE_CURRENT_BINARY_DIR}/mock/tcp_mock.hw_emu.xo)
set(MOCK_TCP_SOURCES
  "hw/mock/tcp.mock.cc")
hlslib_make_paths_absolute(MOCK_TCP_SOURCES ${MOCK_TCP_SOURCES})
add_custom_command(OUTPUT ${MOCK_TCP_XO}
  COMMENT "Compiling mock kernel tcp_mock.hw_emu.xo"
  DEPENDS ${MOCK_TCP_SOURCES}
	COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -c -o ${MOCK_TCP_XO} -k tcp_mock --input_files ${MOCK_TCP_SOURCES})

#
# app kernel
#

set(MOCK_APP_XO ${CMAKE_CURRENT_BINARY_DIR}/mock/app.hw_emu.xo)
set(MOCK_APP_SOURCES
  "examples/static_pages/hw/static_pages.h"
  "examples/static_pages/hw/static_pages.cc"
  "hw/hls/http_lib.h"
  "hw/hls/tcp_utils.h")
hlslib_make_paths_absolute(MOCK_APP_SOURCES ${MOCK_APP_SOURCES})
add_custom_command(OUTPUT ${MOCK_APP_XO}
  COMMENT "Compiling mock kernel app.hw_emu.xo"
  DEPENDS ${MOCK_APP_SOURCES}
	COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -c -o ${MOCK_TCP_XO} -k static_pages --input_files ${MOCK_TCP_SOURCES})

#
# xclbin link
#

set(MOCK_LINK_XCLBIN ${CMAKE_CURRENT_BINARY_DIR}/mock/mock.link.xclbin)
set(MOCK_LINK_SOURCES ${MOCK_HTTP_XO} ${MOCK_TCP_XO} ${MOCK_APP_XO})
add_custom_command(OUTPUT ${MOCK_LINK_XCLBIN}
  COMMENT "Compiling mock kernel app.hw_emu.xo"
  DEPENDS ${MOCK_LINK_SOURCES}
	COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -l -o ${MOCK_LINK_XCLBIN} ${MOCK_LINK_SOURCES}
    --config ${CMAKE_CURRENT_SOURCE_DIR}/hw/mock/connectivity.cfg)

#
# xclbin hw_emu
#

set(MOCK_XCLBIN ${CMAKE_CURRENT_BINARY_DIR}/mock/mock.hw_emu.xclbin)
add_custom_command(OUTPUT ${MOCK_XCLBIN}
  DEPENDS ${MOCK_LINK_XCLBIN}
  COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -p ${MOCK_LINK_XCLBIN} -o ${MOCK_XCLBIN})

#
# host
#

set(MOCK_HOST ${CMAKE_CURRENT_BINARY_DIR}/mock/host)
set(MOCK_HOST_SOURCES
  "hw/mock/host.cc"
  "vitis-network-stack/common/includes/xcl2/xcl2.cpp")
add_executable(${MOCK_HOST} ${MOCK_HOST_SOURCES})
target_link_libraries(${MOCK_HOST} ${Vitis_LIBRARIES})

#
# emconfig
#

set(MOCK_EMU_CFG ${CMAKE_CURRENT_BINARY_DIR}/mock/emconfig.json)
add_custom_command(OUTPUT ${MOCK_EMU_CFG}
  COMMAND emconfigutil --platform ${HTTP_PLATFORM} --od hw_emu)

#
# mock
#

add_custom_target(mock
  DEPENDS ${MOCK_XCLBIN} ${MOCK_HOST} ${MOCK_EMU_CFG})

#
# mock.run
#

add_custom_target(mock.run
  DEPENDS mock
  COMMAND
    cp -f ${MOCK_EMU_CFG} .
	  cp -f ${CMAKE_CURRENT_SOURCE_DIR}/hw/mock/xrt.ini .
	  XCL_EMULATION_MODE=hw_emu ${MOCK_HOST} ${MOCK_XCLBIN})