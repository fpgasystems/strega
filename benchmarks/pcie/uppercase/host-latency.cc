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
    queue = cl::CommandQueue(context, {device}, CL_QUEUE_PROFILING_ENABLE);
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
  file_benchmark << "size, iteration, total_us, input_us, result_us, kernel_us, opencl_us"
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

  for (unsigned int it = 0; it < iterations; it++) {
    uint64_t input_ns;
    uint64_t kernel_ns;
    uint64_t result_ns;
    uint64_t events_ns;
    uint64_t opencl_ns;
    cl::Event  evtKernel;
    cl::Event  evtInput;
    cl::Event  evtResult;

    for (unsigned int i = 0; i < payload_size_bytes; i++) {
        ptr_in[i] = 32 + rand() % 94; // only ASCII char
    }

    ///////////////////////////////////////////////////

    auto start = std::chrono::high_resolution_clock::now();

    queue.enqueueMigrateMemObjects({buffer_in}, 0, NULL, &evtInput);

    queue.enqueueTask(kernel, NULL, &evtKernel);

    queue.enqueueMigrateMemObjects({buffer_out},
      CL_MIGRATE_MEM_OBJECT_HOST,
      NULL,
      &evtResult);

    queue.finish();

    auto finish = std::chrono::high_resolution_clock::now();

    ///////////////////////////////////////////////////

    auto total_ns = finish - start;
    input_ns = get_duration_ns(evtInput);
    result_ns  = get_duration_ns(evtResult);
    kernel_ns  = get_duration_ns(evtKernel);
    events_ns  = input_ns + kernel_ns + result_ns;            
    opencl_ns  = ((uint64_t)total_ns.count() >= events_ns) ? total_ns.count() - events_ns : 0;

    file_benchmark << payload_size_bytes << ","
      << it << ","
      << (uint64_t) total_ns.count() / 1000 << ","
      << input_ns / 1000 << ","
      << result_ns / 1000 << ","
      << kernel_ns / 1000 << ","
      << opencl_ns / 1000 << std::endl;
  }
  file_benchmark.close();
}