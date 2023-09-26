############################## Setting up Project Variables ##############################

TARGET := hw
PLATFORM ?= xilinx_u55c_gen3x16_xdma_3_202210_1

include ../../vitis-network-stack/common/includes/opencl/opencl.mk
xcl2_SRCS:=../../vitis-network-stack/common/includes/xcl2/xcl2.cpp
xcl2_HDRS:=../../vitis-network-stack/common/includes/xcl2/xcl2.hpp
xcl2_CXXFLAGS:=-I../../vitis-network-stack/common/includes/xcl2
CXXFLAGS += $(xcl2_CXXFLAGS)
LDFLAGS += $(xcl2_LDFLAGS) -lOpenCL
HOST_SRCS += $(xcl2_SRCS)

CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -Wall -O0 -g -std=c++17
LDFLAGS += -L$(XILINX_XRT)/lib -pthread -lxrt_coreutil

############################## Setting up Host Variables ##############################
# Include Required Host Source Files
HOST_SRCS += usreqrep.cc
# Host compiler global settings
CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++ -lzmq

EXECUTABLE = ./host

############################## Cleaning Rules ##############################
# Cleaning stuff

.PHONY: host
host: $(EXECUTABLE)

$(EXECUTABLE): $(HOST_SRCS)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)

#
# Docker
#

XCLMGMT_DEVICES :=  $(wildcard /dev/xclmgmt*) $(wildcard /dev/dri/renderD*)
DOCKER_DEVICES = $(foreach d,$(XCLMGMT_DEVICES),--device=$(d):$(d))

.PHONY: build
build: $(EXECUTABLE)
	docker build -t uppercase .

.PHONY: run
run: build
	docker run --rm $(DOCKER_DEVICES) -p 80:80 uppercase