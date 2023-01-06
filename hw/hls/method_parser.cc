/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "method_parser.h"

#define METHOD_COUNT 7
#define METHOD_DEPTH 7

namespace http {

const ap_uint<8> matrix[METHOD_COUNT][METHOD_DEPTH] = {
  {0x43, 0x4F, 0x4E, 0x4E, 0x45, 0x43, 0x54}, // HTTP_METHOD_CONNECT
  {0x44, 0x45, 0x4C, 0x45, 0x54, 0x45, 0x00}, // HTTP_METHOD_DELETE
  {0x47, 0x45, 0x54, 0x00, 0x00, 0x00, 0x00}, // HTTP_METHOD_GET
  {0x48, 0x45, 0x41, 0x44, 0x00, 0x00, 0x00}, // HTTP_METHOD_HEAD
  {0x4F, 0x50, 0x54, 0x49, 0x4F, 0x4E, 0x53}, // HTTP_METHOD_OPTIONS
  {0x50, 0x4F, 0x53, 0x54, 0x00, 0x00, 0x00}, // HTTP_METHOD_POST
  {0x50, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00}, // HTTP_METHOD_PUT
};

HttpMethod method_parser (ap_uint<512> line) {
#pragma HLS inline off

  bool result[METHOD_COUNT];
  for (int i=0; i<METHOD_COUNT; i++) {
#pragma HLS unroll
    bool aux[METHOD_DEPTH];
    for (int j=0; j<METHOD_DEPTH; j++) {
#pragma HLS unroll
      aux[j] = (matrix[i][j] != 0) ? (line.range((j+1)*8-1,j*8) == matrix[i][j]) : true;
    }
    result[i] = true;
    for (int j=0; j<METHOD_DEPTH; j++) {
      result[i] = result[i] & aux[j];
    }
  }

  int value = 0;
  for (int i=0; i<METHOD_COUNT; i++) {
    if (result[i]) {
      value = i + 1;
    }
  }

  return static_cast<HttpMethod>(value);
}

} // namespace http