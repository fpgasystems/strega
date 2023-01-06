/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void req_payload_parser (
  hls::stream<axi_stream_ispt>& input,
  hls::stream<pkt512>& headers,
  hls::stream<pkt512>& body
);

} // namespace http