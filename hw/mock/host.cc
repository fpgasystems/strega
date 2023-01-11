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

  cl::Kernel user_kernel;
  OCL_CHECK(err, user_kernel = cl::Kernel(program, "static_pages", &err));

  uint32_t header_length;
  uint32_t body_pointer;
  uint32_t body_length;

  // Headers
  std::string headers_path = argv[ARGV_HEADERS];
  std::vector<char, aligned_allocator<char>>* headers_data;
  std::ifstream file(headers_path, std::ios::in | std::ios::binary);
  if (file.is_open()) {
    file.seekg(0, std::ios::end);
    header_length = file.tellg();
    std::cout << "headers size: " << header_length << " bytes" << std::endl;

    char* file_buffer = new char[header_length];
    file.seekg(0, std::ios::beg);
    file.read(file_buffer, header_length);

    headers_data = new std::vector<char, aligned_allocator<char>>(header_length);
    memcpy(headers_data->data(), file_buffer, header_length);
    file.close();
    delete [] file_buffer;
  } else {
    std::cerr << "[!] Failed to open HEADERS.txt file" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Body
  std::string body_path = argv[ARGV_BODY];
  std::vector<char, aligned_allocator<char>>* body_data;
  file = std::ifstream(body_path, std::ios::in | std::ios::binary);
  if (file.is_open()) {
    file.seekg(0, std::ios::end);
    body_length = file.tellg();
    std::cout << "body size: " << body_length << " bytes" << std::endl;

    char* file_buffer = new char[body_length];
    file.seekg(0, std::ios::beg);
    file.read(file_buffer, body_length);

    body_data = new std::vector<char, aligned_allocator<char>>(body_length);
    memcpy(body_data->data(), file_buffer, body_length);
    file.close();
    delete [] file_buffer;
  } else {
    std::cerr << "[!] Failed to open BODY.txt file" << std::endl;
    exit(EXIT_FAILURE);
  }

  // std::vector<char, aligned_allocator<char>> buffff(header_length + body_length, 0);
  // OCL_CHECK(err,
  //     cl::Buffer buffer_device(context,
  //       CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
  //       header_length + body_length,
  //       buffff.data(),
  //       &err));

  uint32_t arg = 6;
  body_pointer = header_length;
  // std::cout << "arg[" << arg << "] = " << &buffer_device << std::endl;
  // OCL_CHECK(err, err = user_kernel.setArg(arg++, buffer_device));
  std::cout << "arg[" << arg << "] = " << header_length << std::endl;
  OCL_CHECK(err, err = user_kernel.setArg(arg++, header_length));
  std::cout << "arg[" << arg << "] = " << body_pointer << std::endl;
  OCL_CHECK(err, err = user_kernel.setArg(arg++, body_pointer));
  std::cout << "arg[" << arg << "] = " << body_length << std::endl;
  OCL_CHECK(err, err = user_kernel.setArg(arg++, body_length));

  // cl::Event blocking_call_event;
  // OCL_CHECK(err,
  //   err = q.enqueueWriteBuffer(buffer_device,          // buffer on the FPGA
  //                               CL_TRUE,               // blocking call
  //                               0,                     // buffer offset in bytes
  //                               header_length,         // Size in bytes
  //                               headers_data->data(),  // Pointer to the data to copy
  //                               nullptr, &blocking_call_event));
  // OCL_CHECK(err,
  //   err = q.enqueueWriteBuffer(buffer_device,          // buffer on the FPGA
  //                               CL_TRUE,               // blocking call
  //                               header_length,         // buffer offset in bytes
  //                               body_length,           // Size in bytes
  //                               body_data->data(),     // Pointer to the data to copy
  //                               nullptr, &blocking_call_event));  

  // Launch the Kernel
  std::cout << "enqueue user kernel..." << std::endl;

  OCL_CHECK(err, err = q.enqueueTask(user_kernel));
  OCL_CHECK(err, err = q.finish())

  std::cout << "finished" << std::endl;
}