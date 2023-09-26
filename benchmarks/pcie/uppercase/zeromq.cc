#include "libs/xcl2/xcl2.hpp"
#include "../zmq/zmq.h"

#include <stdlib.h>
#include <vector>
#include <list>
#include <unistd.h> // parameters

#include <endian.h>

#include <thread>

#define DEFAULT_ZMQ_END_POINT "tcp://localhost"

// ZMQ design:
// (arrow indicates connect)
//                                                                    
//                                                               |------  dealer (worker)
//                                                               |
//                                                               |------  dealer (worker)
// client ---[tcp/5555]--->router (proxy) dealer <---[inproc]----|------  dealer (worker)
//                                                               |
//                                                               |------  dealer (worker)
//                                                               |------  dealer (worker)
//
//From left to right: the first router attach in front a frame that contains the id of the request
//                    the dealer then distribute the request on one of the worker (round robin)
//                    finally the worker get the request through one dealer ; we use a dealer here
//                    instead of a REP because it's asynchronous nature allow to tread several requests in parallel
//                    throught batching (the worker has therefore to keep track of the request id and send
//                    it back with answers)

uint64_t get_duration_ns(const cl::Event &event)
{
  uint64_t nstimestart, nstimeend;
  event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_START, &nstimestart);
  event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_END, &nstimeend);
  return (nstimeend - nstimestart);
}

std::string get_timestamp() {
  using namespace std::chrono;

  system_clock::time_point tp = system_clock::now();
  time_t tm = system_clock::to_time_t(tp);
  struct tm stm = *localtime(&tm);

  
  auto dur = tp.time_since_epoch();
  dur -= duration_cast<seconds>(dur);
  
  char timestamp[32] = {0};
  snprintf(timestamp, sizeof(timestamp),
           "%04u-%02u-%02u %02u:%02u:%02u.%03lu",
           stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday,
           stm.tm_hour, stm.tm_min, stm.tm_sec,
           duration_cast<microseconds>(dur).count());
  return std::string(timestamp);
}


struct nfa_buffer_s
{
  uint32_t _cls;
  uint64_t _hash;
  cl::Buffer _buffer;
};

struct invocation_pkg_s
{
  uint32_t payload_size; // in bytes
  cl::Kernel kernel;
  cl::Event evtInput;
  cl::Event evtKernel;
  cl::Event evtOutput;
};

void server (
  const std::string __endpoint,
  zmq::context_t *__zmq_context,
  cl::Context __context,
  cl::Kernel __krnl,
  cl::CommandQueue __queue,
  volatile bool *__quitFlag) {

  // these signals will be handled by the main thread:
  sigset_t signal_mask;
  sigemptyset(&signal_mask);
  sigaddset(&signal_mask, SIGINT);
  sigaddset(&signal_mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);

  cl_int err;

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // ZEROMQ SERVER SETUP                                                                        //
  ////////////////////////////////////////////////////////////////////////////////////////////////

  // construct a ROUTER (async reply) socket and connect to interface
  zmq::socket_t socket(*__zmq_context, zmq::socket_type::dealer);

  // set timeout to 3s (in order to check if the stop flag is set to true)
  socket.set(zmq::sockopt::rcvtimeo, 3000);
  
  // bind to the socket
  socket.connect(__endpoint);

  std::vector<char, aligned_allocator<char>> data_input(
    MAX_NB_QUERIES_IN_BATCH*query_size / CORE_PARAM_GLOBAL_TRAITS::fpga_operand_alignment);

  std::vector<char, aligned_allocator<char>> data_input_next(
                                             MAX_NB_QUERIES_IN_BATCH*query_size /CORE_PARAM_GLOBAL_TRAITS::fpga_operand_alignment);

  uint32_t offset_next = 0;
  std::vector<erbium_ptr_t, aligned_allocator<erbium_ptr_t>> results(MAX_NB_QUERIES_IN_BATCH);

  cl::Buffer main_buffer_in, main_buffer_out;

  OCL_CHECK(err,
    main_buffer_in =
    cl::Buffer(__context,
      CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
      MAX_NB_QUERIES_IN_BATCH*query_size,
      data_input.data(),
      &err));
  
  OCL_CHECK(err,
    main_buffer_out =
    cl::Buffer(__context,
      CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
      MAX_NB_QUERIES_IN_BATCH*sizeof(erbium_ptr_t),
      results.data(),
      &err));

  // Creating Kernel object
  invocation_pkg_s invocation;
  invocation.kernel = __krnl;

  while (true)
  {
    if (*__quitFlag) {
      return;
    }

    auto recv_flags =  zmq::recv_flags::none;
    uint32_t offset = offset_next;
    offset_next = 0;
    
    uint32_t num_queries = 0; // in queries

    ////////////////////////////////////////////////////////////////////////////////////////
    // RECEIVE REQUEST                                                                    //
    ////////////////////////////////////////////////////////////////////////////////////////

    {
      zmq::message_t sender_id;
      zmq::message_t request;

      if (!socket.recv(sender_id, recv_flags)) {
        break;
      }            
      socket.recv(request, zmq::recv_flags::none);
      invocation.payload_size = request.size();

      std::cerr << "Received " << request.to_string() << std::endl;
    }

    // batches aggregation :
    // mapping: vector last adress (+1) -> router id
    // when we receive a new batch ; compute its size; if its size + sizes accumulated > max size (in number of request), send batch and store to 
    // else ; enqueue and return the offset
    
    // if (!create_workload(((char *) request.data()) + 6, 
    //                       num_queries_for_this_batch,
    //                       data_length/num_queries_for_this_batch,
    //                       max_operands_in_batch,
    //                       batching_operands_threshold,
    //                       offset, offset_next,
    //                       data_input,
    //                       data_input_next,
    //                       jdsTranslator
    //                       )) {
    //   continue;
    // }

    //   num_queries += num_queries_for_this_batch;
    //   count_and_zmq_ids.push_back(Zmq_count_and_id(num_queries,sender_id));
    
    ////////////////////////////////////////////////////////////////////////////////////////
    // KERNEL EXECUTION                                                                   //
    ////////////////////////////////////////////////////////////////////////////////////////

    auto start = std::chrono::high_resolution_clock::now();

    cl::Buffer buffer_in, buffer_out;

    cl_buffer_region query_region = {0, kernel.queries_size};
    OCL_CHECK(err, buffer_in =
      main_buffer_in.createSubBuffer(
        CL_MEM_READ_ONLY,
        CL_BUFFER_CREATE_TYPE_REGION,
        &query_region,
        &err));

    cl_buffer_region results_region = {0, kernel.results_cls * C_CACHELINE_SIZE};
    OCL_CHECK(err,buffer_out =
      main_buffer_out.createSubBuffer(
        CL_MEM_WRITE_ONLY,
        CL_BUFFER_CREATE_TYPE_REGION,
        &results_region,
        &err));

    invocation.kernel.setArg(0, buffer_in);  // @pointer to input data
    invocation.kernel.setArg(1, buffer_out); // @pointer to output data
    invocation.kernel.setArg(2, invocation.payload_size); // payload size in bytes

    // load data via PCIe to the FPGA on-board DDR
    __queue.enqueueMigrateMemObjects({buffer_in},
      0 /* 0 means from host*/, NULL, &kernel.evtInput);
    
    // kernel launch
    __queue.enqueueTask(invocation.kernel, NULL, &kernel.evtKernel);

    // load results via PCIe from FPGA on-board DDR
    __queue.enqueueMigrateMemObjects({buffer_out},
      CL_MIGRATE_MEM_OBJECT_HOST, NULL, &kernel.evtOutput);
    
    __queue.finish();

    auto stop = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::ratio<1,1>> elapsed_s = stop-start;
    // std::cout << get_timestamp() << "\t" << "fpga total process duration (seconds): " << elapsed_s.count() << std::endl;

    // send the answers
    for(auto & count_and_id: count_and_zmq_ids)
    {
      // send the reply to the client
      socket.send(count_and_id._id, zmq::send_flags::sndmore);
      socket.send(zmq::buffer((void*)(results.data()), count_and_id._count*2), zmq::send_flags::none);
    }
  }
}

int main(int argc, char *argv[])
{
  ////////////////////////////////////////////////////////////////////////////////////////////////
  // PARAMETERS                                                                                 //
  ////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << ">> Parameters" << std::endl;

  std::string fullpath_bitstream;

  std::string zmq_endpoint(DEFAULT_ZMQ_END_POINT);
  
  char opt;
  while ((opt = getopt(argc, argv, "b:e:")) != -1) {
    switch (opt) {
    case 'b':
      fullpath_bitstream = std::string(optarg);
      break;
    case 'e':
      zmq_endpoint = std::string(optarg);
      break;
    default:
      std::cerr << "Usage: " << argv[0] << "\n"
                << "\t-b  fullpath_bitstream\n"
                << "\t-e  zmq_endpoint\n"
                << "\t-h  help\n"; 
      return EXIT_FAILURE;
    }
  }

  std::cout << "-b fullpath_bitstream: " << fullpath_bitstream      << std::endl;
  std::cout << "-e zmq_endpoint: "       << zmq_endpoint            << std::endl;

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // HARDWARE SETUP                                                                             //
  ////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << ">> Hardware Setup" << std::endl;

  // OpenCL objects
  cl_int err;
  cl::Device device;
  cl::Context context;
  cl::Kernel krnl;
  cl::CommandQueue queue;

  cl::Program program;

  auto devices = xcl::get_xil_devices();
  device = devices[0];
  devices.resize(1);
  auto fileBuf = xcl::read_binary_file(fullpath_bitstream);
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  OCL_CHECK(err, context = cl::Context({device}, NULL, NULL, NULL, &err));
  OCL_CHECK(err, program = cl::Program(context, devices, bins, NULL, &err));

  OCL_CHECK(err, krnl = cl::Kernel(program, "erbium", &err));
  OCL_CHECK(err, queue = cl::CommandQueue(context, {device}, 0, &err));

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // ZMQ SETUP                                                                                  //
  ////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << ">> ZMQ Setup" << std::endl;

  // dealer/router pattern
  zmq::context_t zmq_context(1);

  // Socket facing clients
  zmq::socket_t frontend(zmq_context, zmq::socket_type::router);
  frontend.bind(zmq_endpoint);
  
  // Socket facing services
  zmq::socket_t backend(zmq_context, zmq::socket_type::dealer);
  backend.bind("inproc://dealer");

  // Install the termination signal handlers
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  treads.push_back(
    create_thread(&server,
      "inproc://dealer",
      zmq_context,
      context,
      krnl,
      queue,
      fullpath_dictionary_file,
      sigQuit));

  try {
    zmq::proxy(frontend, backend);
  } catch (zmq::error_t &ex) {
    if (ex.num() != EINTR) {
      throw;
    }
  }
 
  return 0;
}
