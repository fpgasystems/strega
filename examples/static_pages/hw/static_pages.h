/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <hls_stream.h>
#include "../../../hw/hls/http_lib.h"
#include "../../../hw/hls/tcp_utils.h"

void static_pages (
  // HTTP
  hls::stream<http::http_request_spt>& http_request,
  hls::stream<http::pkt512>& http_request_headers,
  hls::stream<http::pkt512>& http_request_body,
  hls::stream<http::http_response_spt>& http_response,
  hls::stream<http::pkt512>& http_response_headers,
  hls::stream<http::pkt512>& http_response_body,
  // Host
  ap_uint<512>* mem_response,
  unsigned int header_length,
  unsigned int body_pointer,
  unsigned int body_length
);
