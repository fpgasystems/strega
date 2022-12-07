/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "request_processor.h"
#include "headline_parser.h"

namespace http {

enum class fsm_state {
  IDLE,
  NOTIFICATION,
  META,
  DATA
};

enum class fsm_state_data {
  HEADLINE,
  PAYLOAD
};

void request_issuer(
  hls::stream<http_headline_ospt>& headline_out,
  hls::stream<http_session_spt>& req_session,
  hls::stream<http_request_spt>& http_request
) {

  if (!headline_out.empty() && !req_session.empty()) {
    http_headline_ospt headline = headline_out.read();
    http_session_spt session = req_session.read();

    http_request_spt request;
    request.meta.method = headline.method;
    request.meta.sessionID = session.id;
    request.method = headline.method;
    request.endpoint = headline.endpoint;

    http_request.write(request);
  }
}

void state_machine(
  hls::stream<pkt128>& tcp_notification,
  hls::stream<pkt32>& tcp_read_request,
  hls::stream<pkt16>& tcp_rx_meta,
  hls::stream<pkt512>& tcp_rx_data,
  hls::stream<http_session_spt>& req_session,
  hls::stream<http_headline_ispt>& headline,
  hls::stream<pkt512>& payload_in
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off
  static fsm_state state = fsm_state::IDLE;
  static fsm_state_data data_state = fsm_state_data::HEADLINE;
  #pragma HLS reset variable=state
  #pragma HLS reset variable=data_state

  switch (state) {
    case fsm_state::IDLE:
    {
      state = (tcp_notification.empty()) ? fsm_state::IDLE : fsm_state::NOTIFICATION;
      break;
    }

    case fsm_state::NOTIFICATION:
    {
      pkt128 raw = tcp_notification.read();
      tcp_notification_pkt notif = tcp_notification_pkt(raw);

      if (notif.length != 0) {
        tcp_read_request_pkt rreq;
        rreq.sessionID = notif.sessionID;
        rreq.length = notif.length;

        http_session_spt session;
        session.id = notif.sessionID;

        tcp_read_request.write(rreq.serialise());
        req_session.write(session);

        state = fsm_state::META;
      } else {
        state = fsm_state::IDLE;
      }
      break;
    }

    case fsm_state::META:
    {
      pkt16 tmp = tcp_rx_meta.read();
      state = fsm_state::DATA;
      break;
    }

    case fsm_state::DATA:
    {
      pkt512 raw = tcp_rx_data.read();

      switch (data_state) {
        case fsm_state_data::HEADLINE:
        {
          http_headline_ispt pkt;
          pkt.line = raw.data;
          headline.write(pkt);
          data_state = fsm_state_data::PAYLOAD;
          break;
        }

        case fsm_state_data::PAYLOAD:
        {
          payload_in.write(raw);
          if (raw.last) {
            data_state = fsm_state_data::HEADLINE;
            state = fsm_state::IDLE;
          }
          break;
        }
      }
      break;
    }
  }
}

void request_processor (
  // TCP/IP
  hls::stream<pkt128>& tcp_notification,
  hls::stream<pkt32>& tcp_read_request,
  hls::stream<pkt16>& tcp_rx_meta,
  hls::stream<pkt512>& tcp_rx_data,
  // INTERNAL
  // APPLICATION
  hls::stream<http_request_spt>& http_request,
  hls::stream<http_request_payload_spt>& http_request_payload
) {
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW disable_start_propagation

  static hls::stream<http_headline_ispt> headline_in("headline_in");
  static hls::stream<http_headline_ospt> headline_out("headline_out");
  static hls::stream<http_session_spt> req_session("req_session");
  static hls::stream<pkt512> payload_in("payload_in");

  state_machine(
    tcp_notification,
    tcp_read_request,
    tcp_rx_meta,
    tcp_rx_data,
    req_session,
    headline_in,
    payload_in);
  
  headline_parser(
    headline_in,
    headline_out);

  request_issuer(
    headline_out,
    req_session,
    http_request);
  
  req_payload_parser(
    payload_in,
    http_request_payload);
}

} // namespace http