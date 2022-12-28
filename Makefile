############################## Setting up Project Variables ##############################

TARGET := hw
PLATFORM ?= xilinx_u55c_gen3x16_xdma_3_202210_1

XCLBIN_NAME = http


TEMP_DIR := ./_x
BUILD_DIR := ./build/ips

VPP := v++
CMD_ARGS = $(BUILD_DIR)/${XCLBIN_NAME}.xclbin

include ./vitis-network-stack/common/includes/opencl/opencl.mk
xcl2_SRCS:=./vitis-network-stack/common/includes/xcl2/xcl2.cpp
xcl2_HDRS:=./vitis-network-stack/common/includes/xcl2/xcl2.hpp
xcl2_CXXFLAGS:=-I./vitis-network-stack/common/includes/xcl2
CXXFLAGS += $(xcl2_CXXFLAGS)
LDFLAGS += $(xcl2_LDFLAGS) -lOpenCL
HOST_SRCS += $(xcl2_SRCS)

CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -Wall -O0 -g -std=c++17
LDFLAGS += -L$(XILINX_XRT)/lib -pthread -lxrt_coreutil

########################## Checking if PLATFORM in allowlist ##########################
PLATFORM_BLOCKLIST += nodma
############################## Setting up Host Variables ##############################
# Include Required Host Source Files
HOST_SRCS += examples/static_pages/sw/host.cc
# Host compiler global settings
CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++

############################## Setting up Kernel Variables ##############################
# Kernel compiler global settings
VPP_FLAGS += -t $(TARGET) --platform $(PLATFORM) --save-temps
ifneq ($(TARGET), hw)
	VPP_FLAGS += -g
endif

EXECUTABLE = ./host
EMCONFIG_DIR = $(TEMP_DIR)
EMU_DIR = $(SDCARD)/data/emulation

############################## Declaring Binary Containers ##############################
BINARY_CONTAINERS += $(BUILD_DIR)/${XCLBIN_NAME}.xclbin
BINARY_CONTAINER_krnl_dump_OBJS += $(BUILD_DIR)/cmac_krnl.xo $(BUILD_DIR)/network_krnl.xo $(BUILD_DIR)/static_pages.xo

############################## Cleaning Rules ##############################
# Cleaning stuff
clean:
	-$(RMDIR) _x .Xil *.log .ipcache

cleanall: clean
	-$(RMDIR) build_dir* sd_card*
	-$(RMDIR) package.*
	-$(RMDIR) _x* *xclbin.run_summary qemu-memory-_* emulation _vimage pl* start_simulation.sh *.xclbin

MODULE = http
TESTBENCH_RUN = vitis_hls -f script.tcl -tclargs ${TARGET} ${MODULE}
TESTBENCH_OUT := $$($(TESTBENCH_RUN) 2>&1 | sed -E -e 's/ERROR/\\\e[1;31mERROR\\\e[0m/gi' -e 's/PASS/\\\e[1;32mPASS\\\e[0m/gi' -e 's/WARNING/\\\e[1;33mWARNING\\\e[0m/gi')

.PHONY: vv
vv:
	@echo -e "${TESTBENCH_OUT}"


.PHONY: http
http: build/ips/http.xo

.PHONY: static_pages
static_pages: build/ips/static_pages.xo

.PHONY: bitstream
bitstream: $(BUILD_DIR)/${XCLBIN_NAME}.xclbin

.PHONY: host
host: $(EXECUTABLE)

build/ips/static_pages.xo: examples/static_pages/hw/static_pages.cc hw/hls/tcp_utils.h hw/hls/http_lib.h
	$(VPP) $(VPP_FLAGS) -c -k static_pages -o build/ips/static_pages.xo --input_files \
		examples/static_pages/hw/static_pages.cc

build/ips/http.xo:
	$(VPP) $(VPP_FLAGS) -c -k wrapper_easynet -o build/ips/http.xo --input_files \
		hw/hls/headline_parser.cc \
		hw/hls/http.cc \
		hw/hls/listen_port.cc \
		hw/hls/method_parser.cc \
		hw/hls/req_payload_parser.cc \
		hw/hls/request_processor.cc \
		hw/hls/response_processor.cc \
		hw/hls/status_code_parser.cc \
		hw/hls/wrapper_easynet.cc

$(EXECUTABLE): $(HOST_SRCS)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)

POSTSYSLINKTCL ?= $(shell readlink -f ./build/vitis-network-stack/scripts/post_sys_link.tcl)

$(BUILD_DIR)/${XCLBIN_NAME}.xclbin: $(BINARY_CONTAINER_krnl_dump_OBJS)
	$(VPP) $(VPP_FLAGS) -l $(VPP_LDFLAGS) --temp_dir $(TEMP_DIR)  -o'$(BUILD_DIR)/${XCLBIN_NAME}.link.xclbin' $(+) \
		--advanced.param compiler.userPostSysLinkTcl=$(POSTSYSLINKTCL) \
		--config connectivity.cfg
	$(VPP) -p $(BUILD_DIR)/${XCLBIN_NAME}.link.xclbin -t $(TARGET) --platform $(PLATFORM) -o $(BUILD_DIR)/${XCLBIN_NAME}.xclbin