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

void state_machine(
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<pkt512>& tcp_tx_data,
  // INTERNAL
  hls::stream<HttpStatus>& resp_status_code,
  hls::stream<http_status_code_ospt>& resp_status_line,
  // APPLICATION
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
  #pragma HLS INLINE off

  static fsm_state state = fsm_state::IDLE;
  #pragma HLS reset variable=state
  static http_response_spt app_response;
  #pragma HLS reset variable=app_response off

  switch (state) {
    case fsm_state::IDLE:
    {
      if (!http_response.empty()) {
        app_response = http_response.read();
        resp_status_code.write(app_response.status_code);
        state = fsm_state::APPLICATION_RESPONSE;
      }
      break;
    }

    case fsm_state::APPLICATION_RESPONSE:
    {
      tcp_xx_req_pkt meta;
      meta.sessionID = app_response.meta.sessionID;
      meta.length = app_response.body_size + app_response.headers_size; // TOOD + status code + end of lines
      tcp_tx_req.write(meta);

      state = fsm_state::META;
      break;
    }

    case fsm_state::META:
    {
      tcp_tx_rsp_pkt tx_status = tcp_tx_rsp.read();
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
      if (!resp_status_line.empty()) {
        http_status_code_ospt tmp = resp_status_line.read();
        pkt512 raw;
        raw.data = tmp.data;
        raw.keep = tmp.keep;
        raw.last = false;
        tcp_tx_data.write(raw);

        state = fsm_state::DATA_HEADERS;
      }
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
      raw.keep(1, 0) = -1;
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

      state = (in.last) ? fsm_state::DATA_EOF : fsm_state::DATA_BODY;
      break;
    }

    case fsm_state::DATA_EOF:
    {
      pkt512 raw;
      raw.data(7,0) = 0x0A;
      raw.data(15,8) = 0x0A;
      raw.keep(1, 0) = -1;
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
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<pkt512>& tcp_tx_data,
  // INTERNAL
  // APPLICATION
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
#pragma HLS DATAFLOW

  static hls::stream<HttpStatus> resp_status_code("resp_status_code");
  static hls::stream<http_status_code_ospt> resp_status_line("resp_status_line");

  #pragma HLS stream variable=resp_status_code type=fifo depth=16
  #pragma HLS stream variable=resp_status_line type=fifo depth=16

  state_machine(
    tcp_tx_req,
    tcp_tx_rsp,
    tcp_tx_data,
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