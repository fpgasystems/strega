/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "http.h"
#include "listen_port.h"
#include "request_processor.h"
#include "response_processor.h"

namespace http {

void http_top (
  // TCP-IP
  hls::stream<ap_uint<16>>& tcp_listen_req,
  hls::stream<bool>& tcp_listen_rsp,
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_xx_req_pkt>& tcp_rx_req,
  hls::stream<ap_uint<16>>& tcp_rx_rsp,
  hls::stream<pkt512>& tcp_rx_data,
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<pkt512>& tcp_tx_data,
  // Application
  hls::stream<http_request_spt>& http_request,
  hls::stream<pkt512>& http_request_headers,
  hls::stream<pkt512>& http_request_body,
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
#pragma HLS INTERFACE axis port=tcp_listen_req
#pragma HLS INTERFACE axis port=tcp_listen_rsp
#pragma HLS INTERFACE axis port=tcp_notification
#pragma HLS INTERFACE axis port=tcp_rx_req
#pragma HLS INTERFACE axis port=tcp_rx_rsp
#pragma HLS INTERFACE axis port=tcp_rx_data
#pragma HLS INTERFACE axis port=tcp_tx_req
#pragma HLS INTERFACE axis port=tcp_tx_rsp
#pragma HLS INTERFACE axis port=tcp_tx_data
#pragma HLS INTERFACE axis port=http_request
#pragma HLS INTERFACE axis port=http_request_headers
#pragma HLS INTERFACE axis port=http_request_body
#pragma HLS INTERFACE axis port=http_response
#pragma HLS INTERFACE axis port=http_response_headers
#pragma HLS INTERFACE axis port=http_response_body

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW disable_start_propagation

  listen_port(
    tcp_listen_req,
    tcp_listen_rsp,
    HTTP_LISTEN_PORT
  );

  request_processor(
    // TCP-IP
    tcp_notification,
    tcp_rx_req,
    tcp_rx_rsp,
    tcp_rx_data,
    // Internal
    // Application
    http_request,
    http_request_headers,
    http_request_body);

  response_processor(
    // TCP-IP
    tcp_tx_req,
    tcp_tx_rsp,
    tcp_tx_data,
    // Internal
    // Application
    http_response,
    http_response_headers,
    http_response_body
  );
}

} // namespace http