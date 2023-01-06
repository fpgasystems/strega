/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

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
  );

} // namespace http