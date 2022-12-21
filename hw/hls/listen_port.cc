/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "listen_port.h"

namespace http {

void listen_request_handler(
  hls::stream<ap_uint<16>>& tcp_listen_req,
  ap_uint<16> port_number
) {
#pragma HLS PIPELINE II=1

  // pkt16 listen_req_pkt;
  // listen_req_pkt.data(15,0) = port_number;
  tcp_listen_req.write(port_number);
}

void listen_response_handler(
  hls::stream<bool>& tcp_listen_rsp
) {
#pragma HLS PIPELINE II=1

  bool listen_rsp_pkt;
  tcp_listen_rsp.read(listen_rsp_pkt);
}

void listen_port (
  hls::stream<ap_uint<16>>& tcp_listen_req,
  hls::stream<bool>& tcp_listen_rsp,
  ap_uint<16> port_number
) {
#pragma HLS dataflow disable_start_propagation

  listen_request_handler(tcp_listen_req, port_number);
  listen_response_handler(tcp_listen_rsp);
}

} // namespace http