/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "response_processor.h"
#include "status_code_parser.h"

namespace http {

enum class fsm_state {
  IDLE,
  APPLICATION_RESPONSE,
  META,
  DATA_HTTP_STATUS,
  DATA_HEADERS,
  DATA_DIVIDER,
  DATA_BODY,
  DATA_EOF
};

enum class fsm_state_data {
  HEADLINE,
  PAYLOAD
};

void state_machine(
  hls::stream<pkt32>& tcp_tx_meta,
  hls::stream<pkt512>& tcp_tx_data,
  hls::stream<pkt64>& tcp_tx_status,
  // INTERNAL
  hls::stream<HttpStatus>& resp_status_code,
  hls::stream<http_status_code_ospt>& resp_status_line,
  // APPLICATION
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off
  static fsm_state state = fsm_state::IDLE;
  ap_uint<16> sessionID;
  #pragma HLS reset variable=state
  #pragma HLS reset variable=sessionID off

  switch (state) {
    case fsm_state::IDLE:
    {
      state = (http_response.empty()) ? fsm_state::IDLE : fsm_state::APPLICATION_RESPONSE;
      break;
    }

    case fsm_state::APPLICATION_RESPONSE:
    {
      http_response_spt raw = http_response.read();
      sessionID = raw.meta.sessionID;
      resp_status_code.write(raw.status_code);
      // raw.http_meta meta;
      // raw.body_size;
      // raw.headers_size;

      state = fsm_state::META;
      break;
    }

    case fsm_state::META:
    {
      tcp_rxtx_request_pkt meta;
      meta.sessionID = sessionID;
      meta.length = 0; // body + headers + status + end of lines
      tcp_tx_meta.write(meta.serialise());

      tcp_tx_status_pkt tx_status = tcp_tx_status.read();
      state = fsm_state::DATA_HTTP_STATUS;
      // TODO handle error
      // switch (tx_status.error) {
      //   case 0:
      //   {
      //     state = fsm_state::DATA;
      //     break;
      //   }
      //   case 1:
      //   {
      //     // TODO
      //     state - fsm_state::ERROR;
      //     break;
      //   }
      //   default:
      //   {
      //     state = fsm_state::META;
      //   }
      // }
      break;
    }

    case fsm_state::DATA_HTTP_STATUS:
    {
      http_status_code_ospt tmp = resp_status_line.read();
      pkt512 raw;
      raw.data = tmp.data;
      raw.keep = tmp.keep;
      raw.last = false;
      tcp_tx_data.write(raw);

      state = fsm_state::DATA_HEADERS;
      break;
    }

    case fsm_state::DATA_HEADERS:
    {
      pkt512 in = http_response_headers.read();
      pkt512 out;
      out.data = in.data;
      out.keep = in.keep;
      out.last = false;
      tcp_tx_data.write(out);

      state = (in.last) ? fsm_state::DATA_DIVIDER : fsm_state::DATA_HEADERS;
      break;
    }

    case fsm_state::DATA_DIVIDER:
    {
      pkt512 raw;
      raw.data(7,0) = 0x0A;
      raw.data(15,8) = 0x0A;
      raw.keep(1, 2) = -1;
      raw.keep(63, 2) = 0;
      raw.last = false;
      tcp_tx_data.write(raw);

      state = fsm_state::DATA_BODY;
      break;
    }

    case fsm_state::DATA_BODY:
    {
      pkt512 in = http_response_body.read();
      pkt512 out;
      out.data = in.data;
      out.keep = in.keep;
      out.last = false;
      tcp_tx_data.write(out);

      state = (in.last) ? fsm_state::DATA_DIVIDER : fsm_state::DATA_HEADERS;
      break;
    }

    case fsm_state::DATA_EOF:
    {
      pkt512 raw;
      raw.data(7,0) = 0x0A;
      raw.data(15,8) = 0x0A;
      raw.keep(1, 2) = -1;
      raw.keep(63, 2) = 0;
      raw.last = true;
      tcp_tx_data.write(raw);

      state = fsm_state::IDLE;
      break;
    }
  }
}

void response_processor (
  // TCP/IP
  hls::stream<pkt32>& tcp_tx_meta,
  hls::stream<pkt512>& tcp_tx_data,
  hls::stream<pkt64>& tcp_tx_status,
  // INTERNAL
  // APPLICATION
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW disable_start_propagation

  static hls::stream<HttpStatus> resp_status_code("resp_status_code");
  static hls::stream<http_status_code_ospt> resp_status_line("resp_status_line");

  state_machine(
    tcp_tx_meta,
    tcp_tx_data,
    tcp_tx_status,
    resp_status_code,
    resp_status_line,
    http_response,
    http_response_headers,
    http_response_body);

  status_code_parser(
    resp_status_code,
    resp_status_line);
}

} // namespace http