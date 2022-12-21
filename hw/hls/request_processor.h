/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void request_processor (
  // TCP/IP
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_rxtx_request_pkt>& tcp_read_request,
  hls::stream<ap_uint<16>>& tcp_rx_meta,
  hls::stream<pkt512>& tcp_rx_data,
  // INTERNAL
  // APPLICATION
  hls::stream<http_request_spt>& http_request,
  hls::stream<pkt512>& http_request_headers,
  hls::stream<pkt512>& http_request_body
);

} // namespace http