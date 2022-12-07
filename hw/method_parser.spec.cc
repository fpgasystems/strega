/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <iostream>
#include <string>
#include "method_parser.h"

using namespace http;

#define TEST_SIZE 8
int main (int argc, char* argv[]) {

  const std::string input[TEST_SIZE] = {
    "CONNECT / HTTP/2",
    "DELETE / HTTP/2",
    "GET / HTTP/2",
    "HEAD / HTTP/2",
    "OPTIONS / HTTP/2",
    "POST / HTTP/2",
    "PUT / HTTP/2",
    "WRONG"
  };
  
  HttpMethod output[TEST_SIZE] = {
    HttpMethod::CONNECT,
    HttpMethod::DELETE,
    HttpMethod::GET,
    HttpMethod::HEAD,
    HttpMethod::OPTIONS,
    HttpMethod::POST,
    HttpMethod::PUT,
    HttpMethod::err,
  };

  bool test = true;
  for (int i=0; i<TEST_SIZE; i++) {
    ap_uint<512> line;
    for (int j=0; j<input[i].size(); j++) {
      line.range((j+1)*8-1, j*8) = input[i].at(j);
    }
    if (method_parser(line) != output[i]) {
      std::cerr << "ERROR: ["<< input[i] << "] failed" << std::endl;
      test = false;
    } else {
      std::cout << "PASS: [" << input[i] << "] parsed as " << (int)output[i] << std::endl;
    }
  }

  return !test;
}
