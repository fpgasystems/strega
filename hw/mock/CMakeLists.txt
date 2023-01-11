# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

#
# http kernel
#

set(MOCK_HTTP_XO ${CMAKE_BINARY_DIR}/mock/http.hw_emu.xo)
set(MOCK_HTTP_SOURCES
  "../hls/headline_parser.cc"
  "../hls/http.cc"
  "../hls/listen_port.cc"
  "../hls/method_parser.cc"
  "../hls/req_payload_parser.cc"
  "../hls/request_processor.cc"
  "../hls/response_aligner.cc"
  "../hls/response_processor.cc"
  "../hls/status_code_parser.cc")
hlslib_make_paths_absolute(MOCK_HTTP_SOURCES ${MOCK_HTTP_SOURCES})
add_custom_command(OUTPUT ${MOCK_HTTP_XO}
  COMMENT "Compiling mock kernel http.hw_emu.xo"
  DEPENDS ${MOCK_HTTP_SOURCES}
  COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -c -o ${MOCK_HTTP_XO} -k http_top --input_files ${MOCK_HTTP_SOURCES})

#
# tcp_mock kernel
#

set(MOCK_TCP_XO ${CMAKE_BINARY_DIR}/mock/tcp_mock.hw_emu.xo)
set(MOCK_TCP_SOURCES
  "tcp.mock.cc")
hlslib_make_paths_absolute(MOCK_TCP_SOURCES ${MOCK_TCP_SOURCES})
add_custom_command(OUTPUT ${MOCK_TCP_XO}
  COMMENT "Compiling mock kernel tcp_mock.hw_emu.xo"
  DEPENDS ${MOCK_TCP_SOURCES}
	COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -c -o ${MOCK_TCP_XO} -k tcp_mock --input_files ${MOCK_TCP_SOURCES})

#
# app kernel
#

set(MOCK_APP_XO ${CMAKE_BINARY_DIR}/mock/app.hw_emu.xo)
set(MOCK_APP_SOURCES
  "../../examples/static_pages/hw/static_pages.h"
  "../../examples/static_pages/hw/static_pages.cc"
  "../hls/http_lib.h"
  "../hls/tcp_utils.h")
hlslib_make_paths_absolute(MOCK_APP_SOURCES ${MOCK_APP_SOURCES})
add_custom_command(OUTPUT ${MOCK_APP_XO}
  COMMENT "Compiling mock kernel app.hw_emu.xo"
  DEPENDS ${MOCK_APP_SOURCES}
	COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -c -o ${MOCK_APP_XO} -k static_pages --input_files ${MOCK_APP_SOURCES})

#
# xclbin link
#

set(MOCK_LINK_XCLBIN ${CMAKE_BINARY_DIR}/mock/mock.link.xclbin)
set(MOCK_LINK_SOURCES ${MOCK_HTTP_XO} ${MOCK_TCP_XO} ${MOCK_APP_XO})
add_custom_command(OUTPUT ${MOCK_LINK_XCLBIN}
  COMMENT "Compiling mock kernel app.hw_emu.xo"
  DEPENDS ${MOCK_LINK_SOURCES}
	COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -g -l -o ${MOCK_LINK_XCLBIN} ${MOCK_LINK_SOURCES}
    --config ${CMAKE_CURRENT_SOURCE_DIR}/connectivity.cfg --vivado.synth.jobs 50 --vivado.impl.jobs 50)

#
# xclbin hw_emu
#

set(MOCK_XCLBIN ${CMAKE_BINARY_DIR}/mock/mock.hw_emu.xclbin)
add_custom_command(OUTPUT ${MOCK_XCLBIN}
  DEPENDS ${MOCK_LINK_XCLBIN}
  COMMAND ${Vitis_COMPILER} --platform ${HTTP_PLATFORM} -t hw_emu -p ${MOCK_LINK_XCLBIN} -o ${MOCK_XCLBIN})

#
# host
#

set(MOCK_HOST mock.host)
set(MOCK_HOST_SOURCES
  "host.cc"
  "../../vitis-network-stack/common/includes/xcl2/xcl2.cpp")
add_executable(${MOCK_HOST} ${MOCK_HOST_SOURCES})
target_link_libraries(${MOCK_HOST} ${Vitis_LIBRARIES})
target_include_directories(${MOCK_HOST} PRIVATE ${Vitis_INCLUDE_DIRS} "../../vitis-network-stack/common/includes/xcl2")

#
# emconfig
#

set(MOCK_EMU_CFG ${CMAKE_BINARY_DIR}/mock/emconfig.json)
add_custom_command(OUTPUT ${MOCK_EMU_CFG}
  COMMAND emconfigutil --platform ${HTTP_PLATFORM} --od ${CMAKE_BINARY_DIR}/mock)

#
# mock
#

add_custom_target(mock
  DEPENDS ${MOCK_XCLBIN} mock.host ${MOCK_EMU_CFG})

#
# mock.run
#

add_custom_target(mock.run
  DEPENDS mock
  COMMAND
    cp -f ${MOCK_EMU_CFG} . &&
	  cp -f ${CMAKE_CURRENT_SOURCE_DIR}/xrt.ini . &&
	  XCL_EMULATION_MODE=hw_emu ${CMAKE_CURRENT_BINARY_DIR}/${MOCK_HOST} ${MOCK_XCLBIN} ${CMAKE_CURRENT_SOURCE_DIR}/../../examples/static_pages/serve-headers.txt ${CMAKE_CURRENT_SOURCE_DIR}/../../examples/static_pages/serve-body.txt)