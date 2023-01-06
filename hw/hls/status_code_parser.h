/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "http_lib.h"

namespace http {

void status_code_parser(
  hls::stream<HttpStatus>& input,
  hls::stream<http_status_code_ospt>& output);

} // namespace http