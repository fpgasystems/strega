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
  // NETWORK KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  // cl::Kernel network_kernel;
  // OCL_CHECK(err, network_kernel = cl::Kernel(program, "tcp_mock", &err));

  // OCL_CHECK(err, err = q.enqueueTask(network_kernel));
  // OCL_CHECK(err, err = q.finish());

  ///////////////////////////////////////////////////////////////////////////////////////
  // HTTP KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////////////
  // USER KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::Kernel user_kernel;
  OCL_CHECK(err, user_kernel = cl::Kernel(program, "static_pages", &err));

  // Request
  uint32_t request_size = 1024; // bytes
  std::vector<char, aligned_allocator<char>> request_data(request_size);
  // OCL_CHECK(err,
  //     cl::Buffer buffer_req(context,
  //       CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
  //       request_size,
  //       request_data.data(),
  //       &err));
  
  uint32_t arg = 6;
  // ap_uint<512>* mem_response
  unsigned int header_length = 128;
  unsigned int body_pointer = 0;
  unsigned int body_length = 256;
  std::cout << "arg[" << arg << "] = " << header_length << std::endl;
  OCL_CHECK(err, err = user_kernel.setArg(arg++, header_length));
  std::cout << "arg[" << arg << "] = " << body_pointer << std::endl;
  OCL_CHECK(err, err = user_kernel.setArg(arg++, body_pointer));
  std::cout << "arg[" << arg << "] = " << body_length << std::endl;
  OCL_CHECK(err, err = user_kernel.setArg(arg++, body_length));

  // Launch the Kernel
  std::cout << "enqueue user kernel..." << std::endl;

  OCL_CHECK(err, err = q.enqueueTask(user_kernel));
  OCL_CHECK(err, err = q.finish())
  // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // OCL_CHECK(err, err = q.enqueueReadBuffer(
  //       buffer_req, // This buffers data will be read
  //       CL_TRUE, // blocking call
  //       0,       // offset
  //       request_size,
  //       request_data.data() // Data will be stored here
  //       ));
  // print output buffer HEX
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (uint32_t i = 0; i < request_size; i++) {
    if ((i % 32) == 0) {
      ss << std::endl;
    }
    ss << ' ' << std::setw(2) << static_cast<unsigned>(request_data[i] & 255);
  }
  std::cout << ss.str();
  std::cout << std::endl;

  std::cout << "finished" << std::endl;
}
