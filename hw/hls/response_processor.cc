/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "response_processor.h"
#include "response_aligner.h"
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
  // INTERNAL
  hls::stream<axi_stream_ispt>& tx_aligner,
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
      meta.length = app_response.body_size + app_response.headers_size + 4; // TOOD + status code + end of lines
      tcp_tx_req.write(meta);

      state = fsm_state::META;
      break;
    }

    case fsm_state::META:
    {
      tcp_tx_rsp_pkt tx_status = tcp_tx_rsp.read();
      state = fsm_state::DATA_HTTP_STATUS;
      // TODO handle error
      break;
    }

    case fsm_state::DATA_HTTP_STATUS:
    {
      if (!resp_status_line.empty()) {
        http_status_code_ospt tmp = resp_status_line.read();
        axi_stream_ispt raw;
        raw.data = tmp.data;
        raw.keep = tmp.keep;
        raw.last = false;
        tx_aligner.write(raw);

        state = fsm_state::DATA_HEADERS;
      }
      break;
    }

    case fsm_state::DATA_HEADERS:
    {
      pkt512 in = http_response_headers.read();
      axi_stream_ispt out;
      out.data = in.data;
      out.keep = in.keep;
      out.last = false;
      tx_aligner.write(out);

      state = (in.last) ? fsm_state::DATA_DIVIDER : fsm_state::DATA_HEADERS;
      break;
    }

    case fsm_state::DATA_DIVIDER:
    {
      axi_stream_ispt raw;
      raw.data.range(7,0) = 0x0A;
      raw.data.range(15,8) = 0x0A;
      raw.keep.range(1, 0) = -1;
      raw.keep.range(63, 2) = 0;
      raw.last = false;
      tx_aligner.write(raw);

      state = fsm_state::DATA_BODY;
      break;
    }

    case fsm_state::DATA_BODY:
    {
      pkt512 in = http_response_body.read();
      axi_stream_ispt out;
      out.data = in.data;
      out.keep = in.keep;
      out.last = false;
      tx_aligner.write(out);

      state = (in.last) ? fsm_state::DATA_EOF : fsm_state::DATA_BODY;
      break;
    }

    case fsm_state::DATA_EOF:
    {
      axi_stream_ispt raw;
      raw.data.range(7,0) = 0x0A;
      raw.data.range(15,8) = 0x0A;
      raw.keep.range(1, 0) = -1;
      raw.keep.range(63, 2) = 0;
      raw.last = true;
      tx_aligner.write(raw);

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

  static hls::stream<axi_stream_ispt> tx_aligner("tx_aligner");
  static hls::stream<HttpStatus> resp_status_code("resp_status_code");
  static hls::stream<http_status_code_ospt> resp_status_line("resp_status_line");

  #pragma HLS stream variable=tx_aligner type=fifo depth=16
  #pragma HLS stream variable=resp_status_code type=fifo depth=16
  #pragma HLS stream variable=resp_status_line type=fifo depth=16
  #pragma HLS aggregate variable=tx_aligner compact=bit
  #pragma HLS aggregate variable=resp_status_code compact=bit
  #pragma HLS aggregate variable=resp_status_line compact=bit

  state_machine(
    tcp_tx_req,
    tcp_tx_rsp,
    tx_aligner,
    resp_status_code,
    resp_status_line,
    http_response,
    http_response_headers,
    http_response_body);

  response_aligner(
    tx_aligner,
    tcp_tx_data
  );

  status_code_parser(
    resp_status_code,
    resp_status_line);
}

} // namespace http