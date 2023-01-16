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
HOST_SRCS += examples/health_check/host.cc
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

############################## Cleaning Rules ##############################
# Cleaning stuff

MODULE = http
TESTBENCH_RUN = vitis_hls -f script.tcl -tclargs ${TARGET} ${MODULE}
TESTBENCH_OUT := $$($(TESTBENCH_RUN) 2>&1 | sed -E -e 's/ERROR/\\\e[1;31mERROR\\\e[0m/gi' -e 's/PASS/\\\e[1;32mPASS\\\e[0m/gi' -e 's/WARNING/\\\e[1;33mWARNING\\\e[0m/gi')

.PHONY: vv
vv:
	@echo -e "${TESTBENCH_OUT}"


.PHONY: host
host: $(EXECUTABLE)

$(EXECUTABLE): $(HOST_SRCS)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)
