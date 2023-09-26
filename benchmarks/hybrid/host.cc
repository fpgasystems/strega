/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

/*#include <hlslib/xilinx/OpenCL.h>
#include <iostream>

#define ARGV_XCLBIN 1

int main (int argc, char **argv) {
  if (argc < ARGV_XCLBIN) {
    std::cout << "Usage: " << argv[0] << " <.xclbin> <response.txt>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string binaryFile = argv[ARGV_XCLBIN];
  uint32_t board_ip = strtol(getenv("FPGA_0_IP_ADDRESS_HEX"), NULL, 16);
  uint32_t board_num = 1;

  std::cout << "FPGA_0_IP_ADDRESS = " << getenv("FPGA_0_IP_ADDRESS") << std::endl;
  std::cout << "FPGA_0_IP_ADDRESS_HEX = " << getenv("FPGA_0_IP_ADDRESS_HEX") << std::endl;

  ///////////////////////////////////////////////////////////////////////////////////////
  // OPENCL
  ///////////////////////////////////////////////////////////////////////////////////////

  hlslib::ocl::Context context;
  auto program = context.MakeProgram(binaryFile);

  unsigned int storage_bytes = 256;
  unsigned int buffer_bytes = 128;

  std::vector<char> buffer_hostA(buffer_bytes, 1); // in
  std::vector<char> buffer_hostB(buffer_bytes, 0); // out
  
  auto storage_device = context.MakeBuffer<char, hlslib::ocl::Access::readWrite>(
    hlslib::ocl::StorageType::HBM, 0, storage_bytes);
  std::cout << "bufferA" << std::endl;
  auto buffer_deviceA = context.MakeBuffer<char, hlslib::ocl::Access::read>(
    hlslib::ocl::StorageType::HBM, 0, buffer_bytes);
  std::cout << "bufferB" << std::endl;  
  auto buffer_deviceB = context.MakeBuffer<char, hlslib::ocl::Access::read>(
    hlslib::ocl::StorageType::HBM, 0, buffer_bytes);
  
  ///////////////////////////////////////////////////////////////////////////////////////
  // KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  unsigned int mode = 1;
  unsigned int req_bytes = storage_bytes;
  unsigned int req_pointer = 0;

  

  std::cout << "enqueue user kernel..." << std::endl;

  try {

    std::cout << "a" << std::endl;

    buffer_deviceA.CopyFromHost(buffer_hostA.begin());

    std::cout << "b" << std::endl;

    auto kernelA = program.MakeKernel("hybrid_static_pages", storage_device, buffer_deviceA,
      mode, req_bytes, req_pointer);

      

    kernelA.ExecuteTask();

    std::cout << "c" << std::endl;

    mode = 2;
    auto kernelB = program.MakeKernel("hybrid_static_pages", storage_device, buffer_deviceB,
      mode, req_bytes, req_pointer);

std::cout << "d" << std::endl;
    kernelB.ExecuteTask();

    std::cout << "e" << std::endl;

    buffer_deviceB.CopyToHost(buffer_hostB.begin());

    std::cout << "f" << std::endl;

  } catch (std::runtime_error const &err) {
    std::cerr << "Execution failed with error: \"" << err.what() << "\"."
              << std::endl;
    return 1;
  }

  bool test = true;
  for(std::size_t i = 0; i < buffer_hostB.size(); ++i) {
    test = test && (buffer_hostA[i] == buffer_hostB[i]);
  }

  std::cout << test << std::endl;

  return test;
}*/

#include <xcl2/xcl2.hpp>
#include <bitset>
#include <vector>
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#define ARGV_XCLBIN 1

int main (int argc, char **argv) {
  if (argc < ARGV_XCLBIN) {
    std::cout << "Usage: " << argv[0] << " <.xclbin> <response.txt>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string binaryFile = argv[ARGV_XCLBIN];

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
  // BENCHMARK KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::Kernel kernel;
  OCL_CHECK(err, kernel = cl::Kernel(program, "hybrid_static_pages", &err));

  std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa1" << std::endl;

  unsigned int storage_bytes = 256;
  unsigned int buffer_bytes = 128;

  std::vector<char, aligned_allocator<char>> storage_host(storage_bytes);
  std::vector<char, aligned_allocator<char>> buffer_hostA(buffer_bytes); // in
  std::vector<char, aligned_allocator<char>> buffer_hostB(buffer_bytes); // out

  unsigned int mode = 1;
  unsigned int req_bytes = storage_bytes;
  unsigned int req_pointer = 0;

  std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa2" << std::endl;

  OCL_CHECK(err,
      cl::Buffer storage_device(context,
        CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
        storage_bytes,
        storage_host.data(),
        &err));

  OCL_CHECK(err,
      cl::Buffer buffer_deviceA(context,
        CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
        buffer_bytes,
        buffer_hostA.data(),
        &err));

  OCL_CHECK(err,
      cl::Buffer buffer_deviceB(context,
        CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
        buffer_bytes,
        buffer_hostB.data(),
        &err));

        std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa3" << std::endl;

  unsigned int arg = 0;
  OCL_CHECK(err, err = kernel.setArg(arg++, storage_device));
  OCL_CHECK(err, err = kernel.setArg(arg++, buffer_deviceA));
  OCL_CHECK(err, err = kernel.setArg(arg++, mode));
  OCL_CHECK(err, err = kernel.setArg(arg++, req_bytes));
  OCL_CHECK(err, err = kernel.setArg(arg++, req_pointer));

  /*cl::Event blocking_call_event;
  OCL_CHECK(err,
              err = q.enqueueWriteBuffer(buffer_deviceA,       // buffer on the FPGA
                                         CL_TRUE,              // blocking call
                                         0,                    // buffer offset in bytes
                                         buffer_bytes,         // Size in bytes
                                         buffer_hostA.data(),  // Pointer to the data to copy
                                         nullptr, &blocking_call_event));*/

std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa4" << std::endl;
  OCL_CHECK(err, err = q.enqueueTask(kernel));
  OCL_CHECK(err, err = q.finish())

std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa5" << std::endl;
  std::cout << "finished" << std::endl;
}