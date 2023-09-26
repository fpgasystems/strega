#include <xcl2.hpp>
#include <bitset>
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#define ARGV_XCLBIN 1
#define ARGV_PAYLOAD_SIZE 2
#define ARGV_ITERATIONS 3

uint64_t get_duration_ns (const cl::Event &event) {
    uint64_t nstimestart, nstimeend;
    event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_START, &nstimestart);
    event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_END, &nstimeend);
    return (nstimeend - nstimestart);
}

int main (int argc, char **argv) {
  if (argc < ARGV_ITERATIONS) {
    std::cout << "Usage: " << argv[0] << " <.xclbin> <payload_size_bytes> <iterations>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string binaryFile = argv[ARGV_XCLBIN];
  uint32_t payload_size_bytes = atoi(argv[ARGV_PAYLOAD_SIZE]);
  uint32_t iterations = atoi(argv[ARGV_ITERATIONS]);

  ///////////////////////////////////////////////////////////////////////////////////////
  // OPENCL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::CommandQueue queue;
  cl::Context context;
  cl::Program program;

  auto devices = xcl::get_xil_devices();
  auto fileBuf = xcl::read_binary_file(binaryFile);
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  bool valid_device = false;
  for (unsigned int i = 0; i < devices.size(); i++) {
    cl_int err;
    auto device = devices[i];
    context = cl::Context({device});
    queue = cl::CommandQueue(context, {device});
    program = cl::Program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      std::cout << "Failed to program device[" << i << "]" << std::endl;
    } else {
      std::cout << "Device[" << i << "]: program successful!" << std::endl;
      valid_device = true;
      break;
    }
  }
  if (!valid_device) {
    std::cout << "[FATAL] Failed to program any device found" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ofstream file_benchmark("uppercase-benchmark.dat");
  file_benchmark << "size, total_us, invoc_per_s"
                 << std::endl;

  ///////////////////////////////////////////////////////////////////////////////////////
  // KERNEL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::Kernel kernel = cl::Kernel(program, "uppercase");

  cl::Buffer buffer_in(context, CL_MEM_READ_ONLY, payload_size_bytes);
  cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, payload_size_bytes);

  kernel.setArg(0, buffer_in);
  kernel.setArg(1, buffer_out);
  kernel.setArg(2, payload_size_bytes);

  char* ptr_in = (char*)queue.enqueueMapBuffer(buffer_in,
        CL_TRUE, CL_MAP_WRITE, 0, payload_size_bytes);

  char* ptr_result = (char*)queue.enqueueMapBuffer(buffer_out,
        CL_TRUE, CL_MAP_READ, 0, payload_size_bytes);

  //
  // ITERATION
  //

  ///////////////////////////////////////////////////


  for (unsigned int ab = 0; ab < iterations; ab++) {

    const auto CALLS = 10000;

    for (unsigned int i = 0; i < payload_size_bytes; i++) {
        ptr_in[i] = 32 + rand() % 94; // only ASCII char
    }

    queue.enqueueMigrateMemObjects({buffer_in}, 0);

    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned int it = 0; it < CALLS; it++)
    {

      queue.enqueueTask(kernel);

      queue.finish();
    }

    auto finish = std::chrono::high_resolution_clock::now();

    queue.enqueueMigrateMemObjects({buffer_out},
        CL_MIGRATE_MEM_OBJECT_HOST);

    ///////////////////////////////////////////////////

    auto total_ns = finish - start;

    file_benchmark << payload_size_bytes << ","
      << (uint64_t) total_ns.count() / 1000 << ","
      << (uint64_t) (CALLS * 1000000) / (total_ns.count() / 100) << std::endl;
  }
  
  file_benchmark.close();
}
