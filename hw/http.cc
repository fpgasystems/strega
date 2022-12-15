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
  // Application
  hls::stream<http_request_spt>& http_request,
  hls::stream<axi_stream_ispt>& http_request_headers,
  hls::stream<axi_stream_ispt>& http_request_body,
  hls::stream<http_response_spt>& http_response,
  hls::stream<axi_stream_ispt>& http_response_headers,
  hls::stream<axi_stream_ispt>& http_response_body
) {
#pragma HLS DATAFLOW disable_start_propagation

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
    // Internal
    // Application
    http_request,
    http_request_headers,
    http_request_body);

  response_processor(
    // TCP-IP
    tcp_tx_meta,
    tcp_tx_data,
    tcp_tx_status,
    // Internal
    // Application
    http_response,
    http_response_headers,
    http_response_body
  );
}

} // namespace http