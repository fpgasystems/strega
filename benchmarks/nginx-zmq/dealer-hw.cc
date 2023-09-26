#include <xcl2.hpp>
#include <zmq.hpp>

#include <iostream>
#include <string>

#include <thread>

#define ARGV_XCLBIN 1
#define MAX_PAYLOAD_SIZE 64 // 100 MiB

void server (
  const std::string __endpoint,
  zmq::context_t *__zmq_context,
  cl::Context __ocl_context,
  cl::Program __program,
  cl::CommandQueue __queue) {

  ///////////////////////////////////////////////////////////////////////////////////////
  // ZEROMQ
  ///////////////////////////////////////////////////////////////////////////////////////

  zmq::socket_t socket(*__zmq_context, zmq::socket_type::dealer);
  
  socket.connect(__endpoint);

  ///////////////////////////////////////////////////////////////////////////////////////
  // KERNEL SETUP
  ///////////////////////////////////////////////////////////////////////////////////////

  cl::Kernel kernel = cl::Kernel(__program, "uppercase");

  cl::Buffer buffer_in(__ocl_context, CL_MEM_READ_ONLY, MAX_PAYLOAD_SIZE);
  cl::Buffer buffer_out(__ocl_context, CL_MEM_WRITE_ONLY, MAX_PAYLOAD_SIZE);

  char* ptr_in = (char*)__queue.enqueueMapBuffer(buffer_in,
        CL_TRUE, CL_MAP_WRITE, 0, MAX_PAYLOAD_SIZE);

  char* ptr_result = (char*)__queue.enqueueMapBuffer(buffer_out,
        CL_TRUE, CL_MAP_READ, 0, MAX_PAYLOAD_SIZE);

  while (true)
  { 
    /////////////////////////////////////////////////////////////////////////////////////
    // RECEIVE REQUEST
    /////////////////////////////////////////////////////////////////////////////////////

    zmq::message_t sender_id;
    zmq::message_t empty;
    zmq::message_t request;

    // std::cout << "..............................." << std::endl;    
    socket.recv(sender_id, zmq::recv_flags::none);
    socket.recv(empty);
    socket.recv(request);
    // std::cout << "Received " << std::string((char*)request.data(), request.size()) << std::endl;

    /////////////////////////////////////////////////////////////////////////////////////
    // KERNEL EXECUTION
    /////////////////////////////////////////////////////////////////////////////////////

    kernel.setArg(0, buffer_in);
    kernel.setArg(1, buffer_out);
    kernel.setArg(2, request.size());

    memcpy((void *) ptr_in, (void *)request.data(), request.size());

    __queue.enqueueMigrateMemObjects({buffer_in}, 0);

    __queue.enqueueTask(kernel);

    __queue.enqueueMigrateMemObjects({buffer_in}, 0);

    // __queue.enqueueMigrateMemObjects({buffer_out},
    //   CL_MIGRATE_MEM_OBJECT_HOST);

    __queue.finish();

    /////////////////////////////////////////////////////////////////////////////////////
    // SEND RESPONSE
    /////////////////////////////////////////////////////////////////////////////////////

    zmq::message_t reply(request.size());
    memcpy((void *) reply.data (), (void *) request.data(), request.size());
    socket.send(sender_id, zmq::send_flags::sndmore);
    socket.send(empty, zmq::send_flags::sndmore);
    socket.send(reply, zmq::send_flags::none);
    // std::cout << "Replied" << std::endl;
  }
}

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
  zmq::socket_t frontend(zmq_context, zmq::socket_type::router);
  frontend.bind("tcp://*:5555");

  zmq::socket_t backend(zmq_context, zmq::socket_type::dealer);
  backend.bind("inproc://dealer");

  auto my_thread = std::thread(
    server,
    "inproc://dealer",
    &zmq_context,
    ocl_context,
    ocl_program,
    queue);
  
  try {
    zmq::proxy(frontend, backend);
  } catch (zmq::error_t &ex) {
    if (ex.num() != EINTR) {
      throw;
    }
  }

  return 0;
}