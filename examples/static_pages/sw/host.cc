
#include <bitset>
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
std::cout << "h" << std::endl << std::flush;
  auto network_kernel = xrt::kernel(device, xclbin_uuid, "network_krnl");
std::cout << "g" << std::endl << std::flush;
  auto network_mem_size_bytes = sizeof(int) * NETWORK_DATA_SIZE;
  std::cout << "f" << std::endl << std::flush;
  auto bank_grp_arg0 = network_kernel.group_id(3);
  auto bank_grp_arg1 = network_kernel.group_id(4);

  std::cout << "a" << std::endl << std::flush;
  auto input_buffer = xrt::bo(device, network_mem_size_bytes, bank_grp_arg0);
  std::cout << "b" << std::endl << std::flush;
  auto output_buffer = xrt::bo(device, network_mem_size_bytes, bank_grp_arg1);
std::cout << "c" << std::endl << std::flush;
  auto run = network_kernel(board_ip, board_num, board_ip, bank_grp_arg0, bank_grp_arg1);
  //run.wait();
  std::cout << "d" << std::endl << std::flush;
  std::cout << "e" << std::endl << std::flush;

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
}
