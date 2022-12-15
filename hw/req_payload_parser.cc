/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "req_payload_parser.h"

namespace http {

enum class fsm_state {
  INPUT_READ,
  HEADER_WRITE,
  BODY_WRITE
};

enum class fsm_data_state {
  HEADER,
  BODY
};

void req_payload_parser(
  hls::stream<axi_stream_ispt>& input,
  hls::stream<axi_stream_ispt>& headers,
  hls::stream<axi_stream_ispt>& body
) {
  static fsm_state state = fsm_state::INPUT_READ;
  static fsm_data_state data_state = fsm_data_state::HEADER;
  static axi_stream_ispt line;
  #pragma HLS reset variable=state
  #pragma HLS reset variable=data_state
  #pragma HLS reset variable=line off

  switch (state) {
    case fsm_state::INPUT_READ:
    {
      if (!input.empty()) {
        line = input.read();
        if (data_state == fsm_data_state::HEADER) {
          state = fsm_state::HEADER_WRITE;
        } else {
          state = fsm_state::BODY_WRITE;
        }
      }
      break;
    }
    case fsm_state::HEADER_WRITE:
    {
      // TODO
      // if detected end of header, switch to body
      headers.write(line);
      state = fsm_state::INPUT_READ;
      data_state = fsm_data_state::BODY;
      break;
    }
    case fsm_state::BODY_WRITE:
    {
      body.write(line);

      state = fsm_state::INPUT_READ;
      if (line.last) {
        data_state = fsm_data_state::HEADER;
      } else {
        data_state = fsm_data_state::BODY;
      }
    }
  }
}

} // namespace http