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
  response.headers_size = 93;
  response.body_size = 16;
  http_response.write(response);
}

void consume_request(
  hls::stream<http::pkt512>& http_request_headers,
  hls::stream<http::pkt512>& http_request_body
) {
  #pragma HLS DATAFLOW

  headers: {
    bool lastH;
    do {
      http::pkt512 tmp = http_request_headers.read();
      lastH = tmp.last;
    } while (!lastH);
  }

  body: {
    bool lastB;
    do {
      http::pkt512 tmp = http_request_body.read();
      lastB = tmp.last;
    } while (!lastB);
  }
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
  // Connection: keep-alive
  tmpH.data( 63,   0) = 0x312E312F50545448;
  tmpH.data(127,  64) = 0x0D4B4F2030303220;
  tmpH.data(191, 128) = 0x746E65746E6F430A;
  tmpH.data(255, 192) = 0x61203A657079542D;
  tmpH.data(319, 256) = 0x69746163696C7070;
  tmpH.data(383, 320) = 0x0D6E6F736A2F6E6F;
  tmpH.data(447, 384) = 0x746E65746E6F430A;
  tmpH.data(511, 448) = 0x3A6874676E654C2D;
  tmpH.keep = -1;
  tmpH.last = 0;
  http_response_headers.write(tmpH);

  tmpH.data( 63,   0) = 0x6e6f430A0d363120;
  tmpH.data(127,  64) = 0x3a6e6f697463656e;
  tmpH.data(191, 128) = 0x6c612d7065656b20;
  tmpH.data(255, 192) = 0x0000000a0d657669;
  tmpH.keep(28,  0) = -1;
  tmpH.keep(63, 29) = 0;
  tmpH.last = 1;
  http_response_headers.write(tmpH);

  // {"status": "OK"}
  tmpB.data( 63,  0) = 0x737574617473227b;
  tmpB.data(127, 64) = 0x7d224b4f22203a22;
  tmpB.keep( 15,  0) = -1;
  tmpB.keep( 63, 16) = 0;
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