/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void http_top (
  // TCP-IP
  hls::stream<ap_uint<16>>& tcp_listen_req,
  hls::stream<bool>& tcp_listen_rsp,
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_xx_req_pkt>& tcp_rx_req,
  hls::stream<ap_uint<16>>& tcp_rx_rsp,
  hls::stream<ap_axiu<512, 0, 0, 0>>& tcp_rx_data,
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<ap_axiu<512, 0, 0, 0>>& tcp_tx_data,
  // Application
  hls::stream<http_request_spt>& http_request,
  hls::stream<pkt512>& http_request_headers,
  hls::stream<pkt512>& http_request_body,
  hls::stream<http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
  );

} // namespace http