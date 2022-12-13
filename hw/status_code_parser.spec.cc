/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <iostream>
#include <string>
#include "status_code_parser.h"

using namespace http;

#define TEST_SIZE 39
int main (int argc, char* argv[]) {
  const HttpStatus input[TEST_SIZE] = {
    HttpStatus::ACCEPTED_202,
    HttpStatus::BAD_GATEWAY_502,
    HttpStatus::BAD_REQUEST_400,
    HttpStatus::CONFLICT_409,
    HttpStatus::CONTINUE_100,
    HttpStatus::CREATED_201,
    HttpStatus::EXPECATION_FAILED_417,
    HttpStatus::FORBIDDEN_403,
    HttpStatus::FOUND_302,
    HttpStatus::GATEWAY_TIMEOUT_504,
    HttpStatus::GONE_410,
    HttpStatus::HTTP_VERSION_NOT_SUPPORTED_505,
    HttpStatus::INTERNAL_SERVER_ERROR_500,
    HttpStatus::LENGTH_REQUIRED_411,
    HttpStatus::METHOD_NOT_ALLOWED_405,
    HttpStatus::MOVED_PERMANENTLY_301,
    HttpStatus::MULTIPLE_CHOICES_300,
    HttpStatus::NO_CONTENT_204,
    HttpStatus::NONAUTHORITATIVE_INFORMATION_203,
    HttpStatus::NOT_ACCEPTABLE_406,
    HttpStatus::NOT_FOUND_404,
    HttpStatus::NOT_IMPLEMENTED_501,
    HttpStatus::NOT_MODIFIED_304,
    HttpStatus::OK_200,
    HttpStatus::PARTIAL_CONTENT_206,
    HttpStatus::PRECONDITION_FAILED_412,
    HttpStatus::PROXY_AUTHENTICATION_REQUIRED_407,
    HttpStatus::REQUEST_ENTITY_TOO_LARGE_413,
    HttpStatus::REQUEST_RANGE_NOT_SATISFIABLE_416,
    HttpStatus::REQUEST_TIMEOUT_408,
    HttpStatus::REQUEST_URI_TOO_LONG_414,
    HttpStatus::RESET_CONTENT_205,
    HttpStatus::SEE_OTHER_303,
    HttpStatus::SERVICE_UNAVAILABLE_503,
    HttpStatus::SWITCHING_PROTOCOLS_101,
    HttpStatus::TEMPORARY_REDIRECT_306,
    HttpStatus::UNAUTHORISED_401,
    HttpStatus::UNSUPPORTED_MEDIA_TYPE_415,
    HttpStatus::USE_PROXY_305
  };
  std::string output[TEST_SIZE] = {
    "HTTP/1.1 202 Accepted",
    "HTTP/1.1 502 Bad Gateway",
    "HTTP/1.1 400 Bad Request",
    "HTTP/1.1 409 Conflict",
    "HTTP/1.1 100 Continue",
    "HTTP/1.1 201 Created",
    "HTTP/1.1 417 Expecation Failed",
    "HTTP/1.1 403 Forbidden",
    "HTTP/1.1 302 Found",
    "HTTP/1.1 504 Gateway Timeout",
    "HTTP/1.1 410 Gone",
    "HTTP/1.1 505 HTTP Version Not Supported",
    "HTTP/1.1 500 Internal Server Error",
    "HTTP/1.1 411 Length Required",
    "HTTP/1.1 405 Method Not Allowed",
    "HTTP/1.1 301 Moved Permanently",
    "HTTP/1.1 300 Multiple Choices",
    "HTTP/1.1 204 No Content",
    "HTTP/1.1 203 Nonauthoritative Information",
    "HTTP/1.1 406 Not Acceptable",
    "HTTP/1.1 404 Not Found",
    "HTTP/1.1 501 Not Implemented",
    "HTTP/1.1 304 Not Modified",
    "HTTP/1.1 200 OK",
    "HTTP/1.1 206 Partial Content",
    "HTTP/1.1 412 Precondition Failed",
    "HTTP/1.1 407 Proxy Authentication Required",
    "HTTP/1.1 413 Request Entity Too Large",
    "HTTP/1.1 416 Request Range Not Satisfiable",
    "HTTP/1.1 408 Request Timeout",
    "HTTP/1.1 414 Request Uri Too Long",
    "HTTP/1.1 205 Reset Content",
    "HTTP/1.1 303 See Other",
    "HTTP/1.1 503 Service Unavailable",
    "HTTP/1.1 101 Switching Protocols",
    "HTTP/1.1 306 Temporary Redirect",
    "HTTP/1.1 401 Unauthorised",
    "HTTP/1.1 415 Unsupported Media Type",
    "HTTP/1.1 305 Use Proxy"
  };

  hls::stream<HttpStatus> stream_in("input");
  hls::stream<http_status_code_ospt> stream_out("output");

  bool test = true;
  for (int i=0; i<TEST_SIZE; i++) {
    stream_in.write(input[i]);
    
    do {
      status_code_parser(stream_in, stream_out);
    } while (stream_out.empty());

    http_status_code_ospt line = stream_out.read();

    bool this_test = true;
    for (int j=0; j<output[i].size(); j++) {
      this_test &= (line.data.range((j+1)*8-1, j*8) == output[i].at(j));
      this_test &= (line.keep.range(j,j) == 1);
    }
    // TODO test LF CR end of line chars
    // TODO test remaining line.keep equal to 0

    if (this_test) {
      std::cout << "PASS: [" << output[i] << "] passed" << std::endl;
    } else {
      std::cerr << "ERROR: ["<< output[i] << "] failed" << std::endl;
    }
    test &= this_test;
  }

  return !test;
}
