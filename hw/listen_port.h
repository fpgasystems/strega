/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include "tcp_utils.h"

namespace http {

void listen_port (
  hls::stream<pkt16>& tcp_listen_req,
  hls::stream<pkt8>& tcp_listen_resp,
  ap_uint<16> port_number
);

} // namespace http