/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "req_payload_parser.h"

namespace http {

enum class fsm_state {
  HEADER_READ,
  HEADER_WRITE,
  BODY_READ,
  BODY_WRITE
};

void req_payload_parser(
  hls::stream<axi_stream_ispt>& input,
  hls::stream<pkt512>& headers,
  hls::stream<pkt512>& body
) {
  static fsm_state state = fsm_state::HEADER_READ;
  static axi_stream_ispt line;
  #pragma HLS reset variable=state
  #pragma HLS reset variable=line off

  switch (state) {
    case fsm_state::HEADER_READ:
    {
      if (!input.empty()) {
        line = input.read();
        state = fsm_state::HEADER_WRITE;
      }
      break;
    }
    case fsm_state::HEADER_WRITE:
    {
      // TODO
      // if detected end of header, switch to body
      line.last = true;
      headers.write(line.serialise());
      state = fsm_state::BODY_READ;
      break;
    }
    case fsm_state::BODY_READ:
    {
      line = input.read();
      state = fsm_state::BODY_WRITE;
      break;
    }
    case fsm_state::BODY_WRITE:
    {
      body.write(line.serialise());

      if (line.last) {
        state = fsm_state::HEADER_READ;
      } else {
        state = fsm_state::BODY_READ;
      }
    }
  }
}

} // namespace http