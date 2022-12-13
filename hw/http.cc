/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "http.h"
#include "listen_port.h"
#include "request_processor.h"
#include "response_processor.h"

namespace http {

void http_top (
  // TCP-IP
  hls::stream<pkt16>& tcp_listen_req,
  hls::stream<pkt8>& tcp_listen_rsp,
  hls::stream<pkt128>& tcp_notification,
  hls::stream<pkt32>& tcp_read_request,
  hls::stream<pkt16>& tcp_rx_meta,
  hls::stream<pkt512>& tcp_rx_data,
  hls::stream<pkt32>& tcp_tx_meta,
  hls::stream<pkt512>& tcp_tx_data,
  hls::stream<pkt64>& tcp_tx_status,
  
  // // TCP-IP not used
  // hls::stream<pkt64>& tcp_open_connection,
  // hls::stream<pkt128>& tcp_open_status,
  // hls::stream<pkt16>& tcp_close_connection,
  // hls::stream<pkt512>& udp_rx,
  // hls::stream<pkt512>& udp_tx,
  // hls::stream<pkt256>& udp_rx_meta,
  // hls::stream<pkt256>& udp_tx_meta,

  // APPLICATION
  hls::stream<http_request_spt>& http_request,
  hls::stream<pkt512>& http_request_headers,
  hls::stream<pkt512>& http_request_body,
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW disable_start_propagation

#pragma HLS INTERFACE axis register port=tcp_listen_req name=tcp_listen_req
#pragma HLS INTERFACE axis register port=tcp_listen_rsp name=tcp_listen_rsp
#pragma HLS INTERFACE axis register port=tcp_notification name=tcp_notification
#pragma HLS INTERFACE axis register port=tcp_read_request name=tcp_read_request
#pragma HLS INTERFACE axis register port=tcp_rx_meta name=tcp_rx_meta
#pragma HLS INTERFACE axis register port=tcp_rx_data name=tcp_rx_data
#pragma HLS INTERFACE axis register port=http_request name=http_request
#pragma HLS INTERFACE axis register port=http_request_headers name=http_request_headers
#pragma HLS INTERFACE axis register port=http_request_body name=http_request_body

  listen_port(
    tcp_listen_req,
    tcp_listen_rsp,
    HTTP_LISTEN_PORT
  );

  request_processor(
    // TCP-IP
    tcp_notification,
    tcp_read_request,
    tcp_rx_meta,
    tcp_rx_data,
    // INTERNAL
    // APPLICATION
    http_request,
    http_request_headers,
    http_request_body);

  response_processor(
    // TCP-IP
    tcp_tx_meta,
    tcp_tx_data,
    tcp_tx_status,
    // INTERNAL
    // APPLICATION
    http_response,
    http_response_headers,
    http_response_body
  );
}

} // namespace http