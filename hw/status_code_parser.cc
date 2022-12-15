/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "status_code_parser.h"

#define PREFIX_LENGTH 9
#define STATUS_CODE_COUNT 46
#define STATUS_CODE_LENGTH 34

namespace http {

const ap_uint<8> prefix[PREFIX_LENGTH] = {0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x20}; // HTTP/1.1 
const ap_uint<8> matrix[STATUS_CODE_COUNT][STATUS_CODE_LENGTH] = {
  {0x31, 0x30, 0x30, 0x20, 0x43, 0x6F, 0x6E, 0x74, 0x69, 0x6E, 0x75, 0x65, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 100 Continue
  {0x31, 0x30, 0x31, 0x20, 0x53, 0x77, 0x69, 0x74, 0x63, 0x68, 0x69, 0x6E, 0x67, 0x20, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x63, 0x6F, 0x6C, 0x73, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 101 Switching Protocols
  {0x32, 0x30, 0x30, 0x20, 0x4F, 0x4B, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 200 OK
  {0x32, 0x30, 0x31, 0x20, 0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 201 Created
  {0x32, 0x30, 0x32, 0x20, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 202 Accepted
  {0x32, 0x30, 0x33, 0x20, 0x4E, 0x6F, 0x6E, 0x61, 0x75, 0x74, 0x68, 0x6F, 0x72, 0x69, 0x74, 0x61, 0x74, 0x69, 0x76, 0x65, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x0A, 0x00}, // 203 Nonauthoritative Information
  {0x32, 0x30, 0x34, 0x20, 0x4E, 0x6F, 0x20, 0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 204 No Content
  {0x32, 0x30, 0x35, 0x20, 0x52, 0x65, 0x73, 0x65, 0x74, 0x20, 0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 205 Reset Content
  {0x32, 0x30, 0x36, 0x20, 0x50, 0x61, 0x72, 0x74, 0x69, 0x61, 0x6C, 0x20, 0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 206 Partial Content
  {0x33, 0x30, 0x30, 0x20, 0x4D, 0x75, 0x6C, 0x74, 0x69, 0x70, 0x6C, 0x65, 0x20, 0x43, 0x68, 0x6F, 0x69, 0x63, 0x65, 0x73, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 300 Multiple Choices
  {0x33, 0x30, 0x31, 0x20, 0x4D, 0x6F, 0x76, 0x65, 0x64, 0x20, 0x50, 0x65, 0x72, 0x6D, 0x61, 0x6E, 0x65, 0x6E, 0x74, 0x6C, 0x79, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 301 Moved Permanently
  {0x33, 0x30, 0x32, 0x20, 0x46, 0x6F, 0x75, 0x6E, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 302 Found
  {0x33, 0x30, 0x33, 0x20, 0x53, 0x65, 0x65, 0x20, 0x4F, 0x74, 0x68, 0x65, 0x72, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 303 See Other
  {0x33, 0x30, 0x34, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x4D, 0x6F, 0x64, 0x69, 0x66, 0x69, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 304 Not Modified
  {0x33, 0x30, 0x35, 0x20, 0x55, 0x73, 0x65, 0x20, 0x50, 0x72, 0x6F, 0x78, 0x79, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 305 Use Proxy
  {0x33, 0x30, 0x36, 0x20, 0x54, 0x65, 0x6D, 0x70, 0x6F, 0x72, 0x61, 0x72, 0x79, 0x20, 0x52, 0x65, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 306 Temporary Redirect
  {0x34, 0x30, 0x30, 0x20, 0x42, 0x61, 0x64, 0x20, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 400 Bad Request
  {0x34, 0x30, 0x31, 0x20, 0x55, 0x6E, 0x61, 0x75, 0x74, 0x68, 0x6F, 0x72, 0x69, 0x73, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 401 Unauthorised
  {0x34, 0x30, 0x33, 0x20, 0x46, 0x6F, 0x72, 0x62, 0x69, 0x64, 0x64, 0x65, 0x6E, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 403 Forbidden
  {0x34, 0x30, 0x34, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x46, 0x6F, 0x75, 0x6E, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 404 Not Found
  {0x34, 0x30, 0x35, 0x20, 0x4D, 0x65, 0x74, 0x68, 0x6F, 0x64, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x41, 0x6C, 0x6C, 0x6F, 0x77, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 405 Method Not Allowed
  {0x34, 0x30, 0x36, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 406 Not Acceptable
  {0x34, 0x30, 0x37, 0x20, 0x50, 0x72, 0x6F, 0x78, 0x79, 0x20, 0x41, 0x75, 0x74, 0x68, 0x65, 0x6E, 0x74, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x52, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x0A}, // 407 Proxy Authentication Required
  {0x34, 0x30, 0x38, 0x20, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x20, 0x54, 0x69, 0x6D, 0x65, 0x6F, 0x75, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 408 Request Timeout
  {0x34, 0x30, 0x39, 0x20, 0x43, 0x6F, 0x6E, 0x66, 0x6C, 0x69, 0x63, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 409 Conflict
  {0x34, 0x31, 0x30, 0x20, 0x47, 0x6F, 0x6E, 0x65, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 410 Gone
  {0x34, 0x31, 0x31, 0x20, 0x4C, 0x65, 0x6E, 0x67, 0x74, 0x68, 0x20, 0x52, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 411 Length Required
  {0x34, 0x31, 0x32, 0x20, 0x50, 0x72, 0x65, 0x63, 0x6F, 0x6E, 0x64, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x46, 0x61, 0x69, 0x6C, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 412 Precondition Failed
  {0x34, 0x31, 0x33, 0x20, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x20, 0x45, 0x6E, 0x74, 0x69, 0x74, 0x79, 0x20, 0x54, 0x6F, 0x6F, 0x20, 0x4C, 0x61, 0x72, 0x67, 0x65, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00}, // 413 Request Entity Too Large
  {0x34, 0x31, 0x34, 0x20, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x20, 0x55, 0x72, 0x69, 0x20, 0x54, 0x6F, 0x6F, 0x20, 0x4C, 0x6F, 0x6E, 0x67, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 414 Request Uri Too Long
  {0x34, 0x31, 0x35, 0x20, 0x55, 0x6E, 0x73, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64, 0x20, 0x4D, 0x65, 0x64, 0x69, 0x61, 0x20, 0x54, 0x79, 0x70, 0x65, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 415 Unsupported Media Type
  {0x34, 0x31, 0x36, 0x20, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x20, 0x52, 0x61, 0x6E, 0x67, 0x65, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x53, 0x61, 0x74, 0x69, 0x73, 0x66, 0x69, 0x61, 0x62, 0x6C, 0x65, 0x0A}, // 416 Request Range Not Satisfiable
  {0x34, 0x31, 0x37, 0x20, 0x45, 0x78, 0x70, 0x65, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x46, 0x61, 0x69, 0x6C, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 417 Expecation Failed
  {0x35, 0x30, 0x30, 0x20, 0x49, 0x6E, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x45, 0x72, 0x72, 0x6F, 0x72, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 500 Internal Server Error
  {0x35, 0x30, 0x31, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x49, 0x6D, 0x70, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 501 Not Implemented
  {0x35, 0x30, 0x32, 0x20, 0x42, 0x61, 0x64, 0x20, 0x47, 0x61, 0x74, 0x65, 0x77, 0x61, 0x79, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 502 Bad Gateway
  {0x35, 0x30, 0x33, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x20, 0x55, 0x6E, 0x61, 0x76, 0x61, 0x69, 0x6C, 0x61, 0x62, 0x6C, 0x65, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 503 Service Unavailable
  {0x35, 0x30, 0x34, 0x20, 0x47, 0x61, 0x74, 0x65, 0x77, 0x61, 0x79, 0x20, 0x54, 0x69, 0x6D, 0x65, 0x6F, 0x75, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 504 Gateway Timeout
  {0x35, 0x30, 0x35, 0x20, 0x48, 0x54, 0x54, 0x50, 0x20, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x20, 0x4E, 0x6F, 0x74, 0x20, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64, 0x0A, 0x00, 0x00, 0x00}  // 505 Http Version Not Supported
};

enum class fsm_state {
  IDLE,
  READ_INPUT,
  WRITE_OUTPUT
};

void status_code_parser(
  hls::stream<HttpStatus>& input,
  hls::stream<http_status_code_ospt>& output
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

  static fsm_state state = fsm_state::IDLE;
  static HttpStatus status_code;
  #pragma HLS reset variable=state

  switch (state) {
    case fsm_state::IDLE:
    {
      state = (input.empty()) ? fsm_state::IDLE : fsm_state::READ_INPUT;
      break;
    }

    case fsm_state::READ_INPUT:
    {
      status_code = input.read();
      state = fsm_state::WRITE_OUTPUT;
      break;
    }
    
    case fsm_state::WRITE_OUTPUT:
    {
      http_status_code_ospt line;
      unsigned int uint_code = static_cast<unsigned int>(status_code);

      // prefix
      for (unsigned int i=0; i<PREFIX_LENGTH; i++) {
        #pragma HLS unroll
        line.data.range((i+1)*8-1, i*8) = prefix[i];
        line.keep.range(i, i) = 1;
      }

      // status code
      for (unsigned int i=PREFIX_LENGTH, j=0; i<PREFIX_LENGTH+STATUS_CODE_LENGTH; i++, j++)
      {
        #pragma HLS unroll
        line.data.range((i+1)*8-1, i*8) = matrix[uint_code][j];
        line.keep.range(i, i) = (matrix[uint_code][j] == 0) ? 0 : 1;
      }

      // remaining
      for (unsigned int i=PREFIX_LENGTH+STATUS_CODE_LENGTH; i<64; i++)
      {
        #pragma HLS unroll
        line.keep.range(i, i) = 0;
      }

      output.write(line);
      state = fsm_state::IDLE;
    }
  }
}

} // namespace http