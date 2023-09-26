#include "libs/xcl2/xcl2.hpp"
#include <zmq.hpp>

#include <algorithm>
#include <iostream>
#include <string>

#include <thread>

void server (
  const std::string __endpoint,
  zmq::context_t *__zmq_context) {

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // ZEROMQ SERVER SETUP                                                                        //
  ////////////////////////////////////////////////////////////////////////////////////////////////

  // construct a ROUTER (async reply) socket and connect to interface
  zmq::socket_t socket(*__zmq_context, zmq::socket_type::dealer);
  
  // bind to the socket
  socket.connect(__endpoint);

  while (true)
  { 
    uint32_t num_queries = 0; // in queries

    ////////////////////////////////////////////////////////////////////////////////////////
    // RECEIVE REQUEST                                                                    //
    ////////////////////////////////////////////////////////////////////////////////////////

    {
      zmq::message_t sender_id;
      zmq::message_t empty;
      zmq::message_t request;
      
      socket.recv(sender_id, zmq::recv_flags::none);
      socket.recv(empty);
      socket.recv(request);
      // std::cout << "Received " << std::string((char*)request.data(), request.size()) << std::endl;

      std::transform(request.data(), request.data() + request.size(), request.data(), ::toupper);

      socket.send(sender_id, zmq::send_flags::sndmore);
      socket.send(empty, zmq::send_flags::sndmore);
      socket.send(request, zmq::send_flags::none);
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // KERNEL EXECUTION                                                                   //
    ////////////////////////////////////////////////////////////////////////////////////////
  }
}

int main()
{
  zmq::context_t zmq_context(1);
  zmq::socket_t frontend(zmq_context, zmq::socket_type::router);
  frontend.bind("tcp://*:5555");

  zmq::socket_t backend(zmq_context, zmq::socket_type::dealer);
  backend.bind("inproc://dealer");

  auto my_thread = std::thread(
    server,
    "inproc://dealer",
    &zmq_context);
  
  try {
    zmq::proxy(frontend, backend);
  } catch (zmq::error_t &ex) {
    if (ex.num() != EINTR) {
      throw;
    }
  }
  // while (true)
  // {
  //   zmq::message_t request;
  //   socket.recv(&request);
  //   std::cout << "Received " << std::string((char*)request.data(), request.size()) << std::endl;
  //   zmq::message_t reply(5);
  //   memcpy ((void *) reply.data (), "World", 5);
  //   socket.send (reply);
    
  // }
  return 0;
}