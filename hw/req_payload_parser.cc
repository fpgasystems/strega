/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

namespace http {

enum class fsm_state {
  INPUT_READ,
  OUTPUT_WRITE
};

void req_payload_parser(
  hls::stream<pkt512>& input,
  hls::stream<http_request_payload_spt>& output
) {
  static fsm_state state = fsm_state::INPUT_READ;
  static pkt512 line;
  #pragma HLS reset variable=state
  #pragma HLS reset variable=line off

  switch (state) {
    case fsm_state::INPUT_READ:
    {
      if (!input.empty()) {
        line = input.read();
        state = fsm_state::OUTPUT_WRITE;
      }
      break;
    }

    case fsm_state::OUTPUT_WRITE:
    {
      // TODO
      http_request_payload_spt result;
      result.type = HttpPayloadType::BODY;
      result.data = line.data;
      output.write(result);
      state = fsm_state::INPUT_READ;
      break;
    }
  }
}

} // namespace http