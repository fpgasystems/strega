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
#define ARGV_HEADERS 2
#define ARGV_BODY 3

int main (int argc, char **argv) {
  if (argc < ARGV_BODY) {
    std::cout << "Usage: " << argv[0] << " <.xclbin> <headers.txt> <body.txt>" << std::endl;
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
    std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
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
  // TCP_MOCK KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  // nothing to do, autorun kernel

  ///////////////////////////////////////////////////////////////////////////////////////
  // STREGA HTTP KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  // nothing to do, autorun kernel

  ///////////////////////////////////////////////////////////////////////////////////////
  // APP KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  // cl::Kernel user_kernel;
  // OCL_CHECK(err, user_kernel = cl::Kernel(program, "static_pages", &err));

  // uint32_t header_length;
  // uint32_t body_pointer;
  // uint32_t body_length;

  // uint32_t arg = 6;
  // std::cout << "arg[" << arg << "] = " << header_length << std::endl;
  // OCL_CHECK(err, err = user_kernel.setArg(arg++, header_length));
  // std::cout << "arg[" << arg << "] = " << body_length << std::endl;
  // OCL_CHECK(err, err = user_kernel.setArg(arg++, body_length));

  // // Launch the Kernel
  // std::cout << "enqueue user kernel..." << std::endl;

  // OCL_CHECK(err, err = q.enqueueTask(user_kernel));
  // OCL_CHECK(err, err = q.finish())

  std::cout << "Only autorun kernels in this mock. Press ENTER to finish the simulation..." << std::endl;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  std::cout << "finished" << std::endl;
}