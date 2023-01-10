/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "headline_parser.h"
#include "method_parser.h"

namespace http {

enum class fsm_state {
  INPUT_READ,
  PARSE_METHOD,
  EXTRACT_ENDPOINT,
  OUTPUT_WRITE
};

void headline_parser (
  hls::stream<http_headline_ispt>& input,
  hls::stream<http_headline_ospt>& output
) {
  static fsm_state state = fsm_state::INPUT_READ;
  static ap_uint<512> line;
  static HttpMethod method;
  static ap_uint<HTTP_SESSION_WIDTH> sessionID;

  #pragma HLS reset variable=state
  #pragma HLS reset variable=line off
  #pragma HLS reset variable=method off
  #pragma HLS reset variable=sessionID off
  
  // static ap_uint<512> endpoint;
  // #pragma HLS reset variable=endpoint off

  switch (state) {
    case fsm_state::INPUT_READ:
    {
      if (!input.empty()) {
        auto raw = input.read();
        sessionID = raw.sessionID;
        line = raw.line;
        state = fsm_state::PARSE_METHOD;
      }
      break;
    }

    case fsm_state::PARSE_METHOD:
    {
      method = method_parser(line);
      // if correct, send to shift endpoint
      // TODO implement if no valid method detected
      state = fsm_state::EXTRACT_ENDPOINT;
      break;
    }

    case fsm_state::EXTRACT_ENDPOINT:
    {
      // endpoint = line;
      // TODO finds endpoint start and endpoint end
      state = fsm_state::OUTPUT_WRITE;
      break;
    }

    case fsm_state::OUTPUT_WRITE:
    {
      http_headline_ospt result;
      result.method = method;
      result.sessionID = sessionID;
      // result.endpoint = endpoint;
      output.write(result);
      state = fsm_state::INPUT_READ;
      break;
    }
  }
}

} // namespace http