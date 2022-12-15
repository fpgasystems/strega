############################## Help Section ##############################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"

############################## Setting up Project Variables ##############################

TARGET := hw
HOST_ARCH := x86
SYSROOT :=
PLATFORM ?= xilinx_u55c_gen3x16_xdma_3_202210_1

XCLBIN_NAME = http


TEMP_DIR := ./_x
BUILD_DIR := ./build/ips

VPP := v++
CMD_ARGS = $(BUILD_DIR)/${XCLBIN_NAME}.xclbin

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
BINARY_CONTAINER_krnl_dump_OBJS += $(BUILD_DIR)/http.xo $(BUILD_DIR)/cmac_krnl.xo $(BUILD_DIR)/network_krnl.xo

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

.PHONY: bitstream
bitstream: $(BUILD_DIR)/${XCLBIN_NAME}.xclbin

.PHONY: host
host: $(EXECUTABLE)

build/ips/http.xo:
	$(VPP) $(VPP_FLAGS) -c -k wrapper_easynet -o build/ips/http.xo --input_files \
		hw/headline_parser.cc \
		hw/http.cc \
		hw/listen_port.cc \
		hw/method_parser.cc \
		hw/req_payload_parser.cc \
		hw/request_processor.cc \
		hw/response_processor.cc \
		hw/status_code_parser.cc \
		hw/wrapper_easynet.cc \
		examples/static_pages/hw/static_pages.cc

$(EXECUTABLE): $(HOST_SRCS)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)

POSTSYSLINKTCL ?= $(shell readlink -f ./build/vitis-network-stack/scripts/post_sys_link.tcl)

$(BUILD_DIR)/${XCLBIN_NAME}.xclbin: $(BINARY_CONTAINER_krnl_dump_OBJS)
	$(VPP) $(VPP_FLAGS) -l $(VPP_LDFLAGS) --temp_dir $(TEMP_DIR)  -o'$(BUILD_DIR)/${XCLBIN_NAME}.link.xclbin' $(+) \
		--advanced.param compiler.userPostSysLinkTcl=$(POSTSYSLINKTCL) \
		--config connectivity.cfg
	$(VPP) -p $(BUILD_DIR)/${XCLBIN_NAME}.link.xclbin -t $(TARGET) --platform $(PLATFORM) -o $(BUILD_DIR)/${XCLBIN_NAME}.xclbin
