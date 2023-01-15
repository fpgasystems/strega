#include <xcl2.hpp>
#include <bitset>
#include <vector>
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#define NETWORK_DATA_SIZE 62500

#define ARGV_XCLBIN 1

int main (int argc, char **argv) {
  if (argc < ARGV_XCLBIN) {
    std::cout << "Usage: " << argv[0] << " <.xclbin>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string binaryFile = argv[ARGV_XCLBIN];
  uint32_t board_ip = strtol(getenv("FPGA_0_IP_ADDRESS_HEX"), NULL, 16);

  std::cout << "FPGA_0_IP_ADDRESS = " << getenv("FPGA_0_IP_ADDRESS") << std::endl;
  std::cout << "FPGA_0_IP_ADDRESS_HEX = " << getenv("FPGA_0_IP_ADDRESS_HEX") << std::endl;

  ///////////////////////////////////////////////////////////////////////////////////////
  // OPENCL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl_int err;
  cl::CommandQueue q;
  cl::Context context;
  cl::Program program;

  auto devices = xcl::get_xil_devices();
  auto fileBuf = xcl::read_binary_file(binaryFile);
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  bool valid_device = false;
  for (unsigned int i = 0; i < devices.size(); i++) {
    auto device = devices[i];
    OCL_CHECK(err, context = cl::Context({device}, NULL, NULL, NULL, &err));
    OCL_CHECK(err, q = cl::CommandQueue(
          context, {device}, CL_QUEUE_PROFILING_ENABLE, &err));
    program = cl::Program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      std::cout << "Failed to program device[" << i << "] with xclbin file!" << std::endl;
    } else {
      std::cout << "Device[" << i << "]: program successful!" << std::endl;
      valid_device = true;
      break;
    }
  }
  if (!valid_device) {
    std::cout << "Failed to program any device found, exit!" << std::endl;
    exit(EXIT_FAILURE);
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  // NETWORK KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::Kernel network_kernel;
  OCL_CHECK(err, network_kernel = cl::Kernel(program, "network_krnl", &err));

  uint32_t board_num = 1;
  auto network_mem_size_bytes = sizeof(int) * NETWORK_DATA_SIZE;
  std::vector<int, aligned_allocator<int>> network_ptr0(NETWORK_DATA_SIZE);
  std::vector<int, aligned_allocator<int>> network_ptr1(NETWORK_DATA_SIZE);

  OCL_CHECK(err, err = network_kernel.setArg(0, board_ip)); // Default IP address
  OCL_CHECK(err, err = network_kernel.setArg(1, board_num)); // Board number
  OCL_CHECK(err, err = network_kernel.setArg(2, board_ip)); // ARP lookup

  OCL_CHECK(err,
      cl::Buffer buffer_r1(context,
        CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
        network_mem_size_bytes,
        network_ptr0.data(),
        &err));
  OCL_CHECK(err,
      cl::Buffer buffer_r2(context,
        CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
        network_mem_size_bytes,
        network_ptr1.data(),
        &err));

  OCL_CHECK(err, err = network_kernel.setArg(3, buffer_r1));
  OCL_CHECK(err, err = network_kernel.setArg(4, buffer_r2));

  OCL_CHECK(err, err = q.enqueueTask(network_kernel));
  OCL_CHECK(err, err = q.finish());

  std::cout << "Running autorun application HEALTH CHECK" << std::endl;
}
