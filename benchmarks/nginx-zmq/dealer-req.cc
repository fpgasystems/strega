#include <xcl2.hpp>
#include <zmq.hpp>

#include <iostream>
#include <string>

#include <thread>

#define ARGV_XCLBIN 1
#define MAX_PAYLOAD_SIZE 64 // 100 MiB

int main (int argc, char **argv)
{
  if (argc < ARGV_XCLBIN) {
    std::cout << "Usage: " << argv[0] << " <.xclbin>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string binaryFile = argv[ARGV_XCLBIN];

  ///////////////////////////////////////////////////////////////////////////////////////
  // OPENCL
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::CommandQueue queue;
  cl::Context ocl_context;
  cl::Program ocl_program;

  {
    auto devices = xcl::get_xil_devices();
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
      cl_int err;
      ocl_context = cl::Context({devices[i]});
      queue = cl::CommandQueue(ocl_context, {devices[i]});
      ocl_program = cl::Program(ocl_context, {devices[i]}, bins, NULL, &err);
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
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  // ZEROMQ SETUP
  ///////////////////////////////////////////////////////////////////////////////////////

  zmq::context_t zmq_context(1);
  zmq::socket_t socket(zmq_context, zmq::socket_type::rep);
  socket.bind("tcp://*:5555");

  ///////////////////////////////////////////////////////////////////////////////////////
  // KERNEL SETUP
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::Kernel kernel = cl::Kernel(ocl_program, "uppercase");

  cl::Buffer buffer_in(ocl_context, CL_MEM_READ_ONLY, MAX_PAYLOAD_SIZE);
  cl::Buffer buffer_out(ocl_context, CL_MEM_WRITE_ONLY, MAX_PAYLOAD_SIZE);

  char* ptr_in = (char*)queue.enqueueMapBuffer(buffer_in,
        CL_TRUE, CL_MAP_WRITE, 0, MAX_PAYLOAD_SIZE);

  char* ptr_result = (char*)queue.enqueueMapBuffer(buffer_out,
        CL_TRUE, CL_MAP_READ, 0, MAX_PAYLOAD_SIZE);

  while (true)
  {
    /////////////////////////////////////////////////////////////////////////////////////
    // RECEIVE REQUEST
    /////////////////////////////////////////////////////////////////////////////////////

    zmq::message_t request;
    socket.recv(request);

    /////////////////////////////////////////////////////////////////////////////////////
    // KERNEL EXECUTION
    /////////////////////////////////////////////////////////////////////////////////////

    kernel.setArg(0, buffer_in);
    kernel.setArg(1, buffer_out);
    kernel.setArg(2, request.size());

    memcpy((void *) ptr_in, (void *)request.data(), request.size());

    queue.enqueueMigrateMemObjects({buffer_in}, 0);

    queue.enqueueTask(kernel);

    queue.enqueueMigrateMemObjects({buffer_in}, 0);

    // queue.enqueueMigrateMemObjects({buffer_out},
    //   CL_MIGRATE_MEM_OBJECT_HOST);

    queue.finish();

    /////////////////////////////////////////////////////////////////////////////////////
    // SEND RESPONSE
    /////////////////////////////////////////////////////////////////////////////////////

    zmq::message_t reply(request.size());
    memcpy((void *) reply.data (), (void *) request.data(), request.size());
    socket.send(reply, zmq::send_flags::none);
  }

  return 0;
}