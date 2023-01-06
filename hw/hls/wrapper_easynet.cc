/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "http.h"
#include "../../vitis-network-stack/kernel/common/include/communication.hpp"

void wrapper_easynet(
  // TCP-IP
  hls::stream<pkt16>& m_axis_tcp_listen_port,
  hls::stream<pkt8>& s_axis_tcp_port_status,
  hls::stream<pkt128>& s_axis_tcp_notification,
  hls::stream<pkt32>& m_axis_tcp_read_pkg,
  hls::stream<pkt16>& s_axis_tcp_rx_meta,
  hls::stream<pkt512>& s_axis_tcp_rx_data,
  hls::stream<pkt32>& m_axis_tcp_tx_meta,
  hls::stream<pkt512>& m_axis_tcp_tx_data,
  hls::stream<pkt64>& s_axis_tcp_tx_status,
  hls::stream<pkt512>& s_axis_udp_rx,               // NOT USED
  hls::stream<pkt512>& m_axis_udp_tx,               // NOT USED
  hls::stream<pkt256>& s_axis_udp_rx_meta,          // NOT USED
  hls::stream<pkt256>& m_axis_udp_tx_meta,          // NOT USED
  hls::stream<pkt64>& m_axis_tcp_open_connection,   // NOT USED
  hls::stream<pkt128>& s_axis_tcp_open_status,      // NOT USED
  hls::stream<pkt16>& m_axis_tcp_close_connection,  // NOT USED
  // Application
  hls::stream<http::http_request_spt>& http_request,
  hls::stream<pkt512>& http_request_headers,
  hls::stream<pkt512>& http_request_body,
  hls::stream<http::http_response_spt>& http_response,
  hls::stream<pkt512>& http_response_headers,
  hls::stream<pkt512>& http_response_body
) {
#pragma HLS INTERFACE axis name=m_axis_tcp_listen_port
#pragma HLS INTERFACE axis name=s_axis_tcp_port_status
#pragma HLS INTERFACE axis name=s_axis_tcp_notification
#pragma HLS INTERFACE axis name=m_axis_tcp_read_pkg
#pragma HLS INTERFACE axis name=s_axis_tcp_rx_meta
#pragma HLS INTERFACE axis name=s_axis_tcp_rx_data
#pragma HLS INTERFACE axis name=http_request
#pragma HLS INTERFACE axis name=http_request_headers
#pragma HLS INTERFACE axis name=http_request_body
#pragma HLS INTERFACE axis name=http_response
#pragma HLS INTERFACE axis name=http_response_headers
#pragma HLS INTERFACE axis name=http_response_body

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW disable_start_propagation

  http::http_top (
    m_axis_tcp_listen_port,
    s_axis_tcp_port_status,
    s_axis_tcp_notification,
    m_axis_tcp_read_pkg,
    s_axis_tcp_rx_meta,
    s_axis_tcp_rx_data,
    m_axis_tcp_tx_meta,
    m_axis_tcp_tx_data,
    s_axis_tcp_tx_status,
    http_request,
    http_request_headers,
    http_request_body,
    http_response,
    http_response_headers,
    http_response_body);

  tie_off_tcp_open_connection(
    m_axis_tcp_open_connection,
    s_axis_tcp_open_status);

  tie_off_udp(
    s_axis_udp_rx,
    m_axis_udp_tx,
    s_axis_udp_rx_meta,
    m_axis_udp_tx_meta);

  tie_off_tcp_close_con(
    m_axis_tcp_close_connection);
}