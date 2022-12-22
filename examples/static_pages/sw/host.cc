/*#include <bitset>
#include <vector>
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include <xrt/xrt_device.h>
#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_kernel.h>
#include <experimental/xrt_ip.h>

#define NETWORK_DATA_SIZE 62500

#define ARGV_XCLBIN 1

int main (int argc, char **argv) {
  if (argc < ARGV_XCLBIN) {
    std::cout << "Usage: " << argv[0] << " <.xclbin>" << std::endl;
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

  auto device = xrt::device(0);
  auto xclbin_uuid = device.load_xclbin(binaryFile);
  std::cout << "device name:     " << device.get_info<xrt::info::device::name>() << std::endl;

  ///////////////////////////////////////////////////////////////////////////////////////
  // NETWORK KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  auto network_kernel = xrt::kernel(device, xclbin_uuid, "network_krnl");

  auto network_mem_size_bytes = sizeof(int) * NETWORK_DATA_SIZE;
  auto bank_grp_arg0 = network_kernel.group_id(3);
  auto bank_grp_arg1 = network_kernel.group_id(4);
  auto input_buffer = xrt::bo(device, network_mem_size_bytes, bank_grp_arg0);
  auto output_buffer = xrt::bo(device, network_mem_size_bytes, bank_grp_arg1);

  auto run = network_kernel(board_ip, board_num, board_ip, bank_grp_arg0, bank_grp_arg1);
  run.wait();

  ///////////////////////////////////////////////////////////////////////////////////////
  // USER KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  std::cout << "enqueue user kernel..." << std::endl;
  unsigned int header_length = 128;
  unsigned int body_pointer = 0;
  unsigned int body_length = 256;
  auto app_kernel = xrt::kernel(device, xclbin_uuid, "static_pages");

  auto run2 = network_kernel(header_length, body_pointer, body_length);
  run2.wait();

  std::cout << "finished" << std::endl;
}*/

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
  uint32_t board_ip = strtol(getenv("FPGA_0_IP_ADDRESS_HEX"), NULL, 16);
  uint32_t board_num = 1;

  std::cout << "FPGA_0_IP_ADDRESS = " << getenv("FPGA_0_IP_ADDRESS") << std::endl;
  std::cout << "FPGA_0_IP_ADDRESS_HEX = " << getenv("FPGA_0_IP_ADDRESS_HEX") << std::endl;

  ///////////////////////////////////////////////////////////////////////////////////////
  // OPENCL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl_int err;
  cl::CommandQueue q;
  cl::Context context;
  cl::Kernel user_kernel;
  cl::Kernel network_kernel;

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
    cl::Program program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      std::cout << "Failed to program device[" << i << "] with xclbin file!" << std::endl;
    } else {
      std::cout << "Device[" << i << "]: program successful!" << std::endl;
      OCL_CHECK(err, network_kernel = cl::Kernel(program, "network_krnl", &err));
      OCL_CHECK(err, user_kernel = cl::Kernel(program, "static_pages", &err));
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

  ///////////////////////////////////////////////////////////////////////////////////////
  // USER KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

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

  std::cout << "finished" << std::endl;
}
