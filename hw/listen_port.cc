/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "listen_port.h"

namespace http {

void listen_request_handler(
  hls::stream<pkt16>& tcp_listen_req,
  ap_uint<16> port_number
) {
#pragma HLS PIPELINE II=1

  pkt16 listen_req_pkt;
  listen_req_pkt.data(15,0) = port_number;
  tcp_listen_req.write(listen_req_pkt);
}

void listen_response_handler(
  hls::stream<pkt8>& tcp_listen_rsp
) {
#pragma HLS PIPELINE II=1

  pkt8 listen_rsp_pkt;
  tcp_listen_rsp.read(listen_rsp_pkt);
}

void listen_port (
  hls::stream<pkt16>& tcp_listen_req,
  hls::stream<pkt8>& tcp_listen_resp,
  ap_uint<16> port_number
) {
#pragma HLS dataflow disable_start_propagation

  listen_request_handler(tcp_listen_req, port);
  listen_response_handler(tcp_listen_resp, port);
}

} // namespace http