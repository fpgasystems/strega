/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "request_processor.h"
#include "req_payload_parser.h"
#include "headline_parser.h"

namespace http {

enum class fsm_state {
  IDLE,
  NOTIFICATION,
  HANDSHAKE,
  DATA_HEADLINE,
  DATA_PAYLOAD
};

void request_issuer(
  hls::stream<http_headline_ospt>& headline_out,
  hls::stream<http_request_spt>& http_request
) {
  #pragma HLS INLINE off

  if (!headline_out.empty()) {
    http_headline_ospt headline = headline_out.read();

    http_request_spt request;
    request.meta.method = headline.method;
    request.meta.sessionID = headline.sessionID;
    request.method = headline.method;
    // request.endpoint = headline.endpoint;

    http_request.write(request);
  }
}

void state_machine(
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_xx_req_pkt>& tcp_rx_req,
  hls::stream<ap_uint<16>>& tcp_rx_rsp,
  hls::stream<pkt512>& tcp_rx_data,
  hls::stream<http_headline_ispt>& headline,
  hls::stream<axi_stream_ispt>& payload_in
) {
  #pragma HLS INLINE off

  static fsm_state state = fsm_state::IDLE;
  static ap_uint<HTTP_SESSION_WIDTH> sessionID;
  #pragma HLS reset variable=state
  #pragma HLS reset variable=sessionID off

  switch (state) {
    case fsm_state::IDLE:
    {
      state = (tcp_notification.empty()) ? fsm_state::IDLE : fsm_state::NOTIFICATION;
      break;
    }

    case fsm_state::NOTIFICATION:
    {
      tcp_notification_pkt notif = tcp_notification.read();

      if (notif.length != 0) {
        tcp_xx_req_pkt rx_req;
        rx_req.sessionID = notif.sessionID;
        rx_req.length = notif.length;

        sessionID = notif.sessionID;

        tcp_rx_req.write(rx_req);

        state = fsm_state::HANDSHAKE;
      } else {
        state = fsm_state::IDLE;
      }
      break;
    }

    case fsm_state::HANDSHAKE:
    {
      ap_uint<16> tmp = tcp_rx_rsp.read();
      state = fsm_state::DATA_HEADLINE;
      break;
    }

    case fsm_state::DATA_HEADLINE:
    {
      pkt512 raw = tcp_rx_data.read();      
      http_headline_ispt pkt;
      pkt.line = raw.data;
      pkt.sessionID = sessionID;
      headline.write(pkt);
      state = fsm_state::DATA_PAYLOAD;
      
      break;
    }

    case fsm_state::DATA_PAYLOAD:
    {
      pkt512 raw = tcp_rx_data.read();
      payload_in.write(raw);
      if (raw.last) {
        state = fsm_state::IDLE;
      }
      break;
    }
  }
}

void request_processor (
  // TCP/IP
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_xx_req_pkt>& tcp_rx_req,
  hls::stream<ap_uint<16>>& tcp_rx_rsp,
  hls::stream<pkt512>& tcp_rx_data,
  // INTERNAL
  // APPLICATION
  hls::stream<http_request_spt>& http_request,
  hls::stream<pkt512>& http_request_headers,
  hls::stream<pkt512>& http_request_body
) {
  #pragma HLS DATAFLOW

  static hls::stream<http_headline_ispt> headline_in("headline_in");
  static hls::stream<http_headline_ospt> headline_out("headline_out");
  static hls::stream<axi_stream_ispt> payload_in("payload_in");

  #pragma HLS stream variable=headline_in type=fifo depth=16
  #pragma HLS stream variable=headline_out type=fifo depth=1
  #pragma HLS stream variable=payload_in type=fifo depth=16
  #pragma HLS aggregate variable=headline_in compact=bit
  #pragma HLS aggregate variable=headline_out compact=bit
  #pragma HLS aggregate variable=payload_in compact=bit

  state_machine(
    tcp_notification,
    tcp_rx_req,
    tcp_rx_rsp,
    tcp_rx_data,
    headline_in,
    payload_in);
  
  headline_parser(
    headline_in,
    headline_out);

  request_issuer(
    headline_out,
    http_request);
  
  req_payload_parser(
    payload_in,
    http_request_headers,
    http_request_body);
}

} // namespace http