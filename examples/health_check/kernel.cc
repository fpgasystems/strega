/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <hls_stream.h>
#include "../../hw/hls/http_lib.h"
#include "../../hw/hls/tcp_utils.h"

void process_notify(
  hls::stream<http::http_request_spt>& http_request,
  hls::stream<http::http_response_spt>& http_response
) {
  #pragma HLS PIPELINE II=1

  http::http_request_spt request;
  http_request.read(request);

  http::http_response_spt response;
  response.meta = request.meta;
  response.status_code = http::HttpStatus::OK_200;
  response.headers_size = 69;
  response.body_size = 16;
  http_response.write(response);
}

void consume_request(
  hls::stream<http::pkt512>& http_request_headers,
  hls::stream<http::pkt512>& http_request_body
) {
  #pragma HLS PIPELINE II=1

  bool last;
  do {
    http::pkt512 tmp = http_request_headers.read();
    last = tmp.last;
  } while (!last);

  do {
    http::pkt512 tmp = http_request_body.read();
    last = tmp.last;
  } while (!last);
}

void produce_response(
  hls::stream<http::pkt512>& http_response_headers,
  hls::stream<http::pkt512>& http_response_body
) {
  #pragma HLS PIPELINE II=1

  http::pkt512 tmpH;
  http::pkt512 tmpB;

  // HTTP/1.1 200 OK
  // Content-Type: application/json
  // Content-Length: 16
  tmpH.data( 63,   0) = 0x485454502F312E31;
  tmpH.data(127,  64) = 0x20323030204F4B0D;
  tmpH.data(191, 128) = 0x0A436F6E74656E74;
  tmpH.data(255, 192) = 0x2D547970653A2061;
  tmpH.data(319, 256) = 0x70706C6963617469;
  tmpH.data(383, 320) = 0x6F6E2F6A736F6E0D;
  tmpH.data(447, 384) = 0x0A436F6E74656E74;
  tmpH.data(511, 448) = 0x2D4C656E6774683A;
  tmpH.keep = -1;
  tmpH.last = 0;
  http_response_headers.write(tmpH);

  tmpH.data(39, 0) = 0x2031360D0A;
  tmpH.keep( 4, 0) = -1;
  tmpH.last = 1;
  http_response_headers.write(tmpH);
      
  // {"status": "OK"}
  tmpB.data( 63,  0) = 0x7b22737461747573;
  tmpB.data(127, 64) = 0x223a20224f4b227d;
  tmpB.keep( 15,  0) = -1;
  tmpB.keep(511, 16) = 0;
  tmpB.last = 1;
  http_response_body.write(tmpB);
}

void health_check (
  // HTTP
  hls::stream<http::http_request_spt>& http_request,
  hls::stream<http::pkt512>& http_request_headers,
  hls::stream<http::pkt512>& http_request_body,
  hls::stream<http::http_response_spt>& http_response,
  hls::stream<http::pkt512>& http_response_headers,
  hls::stream<http::pkt512>& http_response_body
) {
  #pragma HLS INTERFACE axis port=http_request
  #pragma HLS INTERFACE axis port=http_request_headers
  #pragma HLS INTERFACE axis port=http_request_body
  #pragma HLS INTERFACE axis port=http_response
  #pragma HLS INTERFACE axis port=http_response_headers
  #pragma HLS INTERFACE axis port=http_response_body

  #pragma HLS INTERFACE ap_ctrl_none port=return
  #pragma HLS DATAFLOW

  process_notify(
    http_request,
    http_response
  );

  consume_request(
    http_request_headers,
    http_request_body
  );

  produce_response(
    http_response_headers,
    http_response_body
  );
}