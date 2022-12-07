/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void request_processor (
  hls::stream<pkt512>& input,
  hls::stream<http_request_payload_spt>& output
);

} // namespace http