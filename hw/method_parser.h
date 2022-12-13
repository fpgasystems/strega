/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include "http_lib.h"

namespace http {

HttpMethod method_parser (ap_uint<512> line);

} // namespace http