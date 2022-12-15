/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void response_processor (
  // TCP/IP
  hls::stream<pkt32>& tcp_tx_meta,
  hls::stream<pkt512>& tcp_tx_data,
  hls::stream<pkt64>& tcp_tx_status,
  // INTERNAL
  // APPLICATION
  hls::stream<http_response_spt>& http_response,
  hls::stream<axi_stream_ispt>& http_response_headers,
  hls::stream<axi_stream_ispt>& http_response_body);

} // namespace http