/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void response_processor (
  // TCP/IP
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<pkt512>& tcp_tx_data,
  // INTERNAL
  // APPLICATION
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body);

} // namespace http