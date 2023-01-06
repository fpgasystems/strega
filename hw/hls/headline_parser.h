/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"

namespace http {

void headline_parser(
  hls::stream<http_headline_ispt>& input,
  hls::stream<http_headline_ospt>& output);

} // namespace http