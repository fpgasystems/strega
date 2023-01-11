/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"
#include "tcp_utils.h"

namespace http {

void response_aligner (
  hls::stream<axi_stream_ispt>& tx_aligner,
  hls::stream<pkt512>& tcp_tx_data);

} // namespace http