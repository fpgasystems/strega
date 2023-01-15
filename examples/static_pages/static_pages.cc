/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "static_pages.h"

struct internal_pkt {
  http::http_meta meta;
  bool service_running;
};

void process_request(
  hls::stream<http::http_request_spt>& http_request,
  hls::stream<http::pkt512>& http_request_headers,
  hls::stream<http::pkt512>& http_request_body,
  hls::stream<internal_pkt>& strm_internal
) {
  bool service_running = true;
  do {
    if (!http_request.empty()) {
      http::http_request_spt request = http_request.read();
      service_running = (request.method != http::HttpMethod::DELETE);

      internal_pkt internal;
      internal.meta = request.meta;
      internal.service_running = service_running;
      strm_internal.write(internal);

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
  } while (service_running);
}

void write_tx_notify(
  hls::stream<http::http_response_spt>& http_response,
  http::http_meta meta,
  unsigned int header_length,
  unsigned int body_length
) {
  #pragma HLS PIPELINE II=1
  #pragma HLS inline off

  http::http_response_spt response;
  response.meta = meta;
  response.status_code = http::HttpStatus::OK_200;
  response.body_size = body_length;
  response.headers_size = header_length;
  http_response.write(response);
}

void write_tx_data(
  hls::stream<http::pkt512>& http_response_headers,
  hls::stream<http::pkt512>& http_response_body,
  ap_uint<512>* mem_response,
  unsigned int header_length,
  unsigned int body_length
) {
  #pragma HLS PIPELINE II=1
  #pragma HLS inline off

  http::pkt512 tmp;
  unsigned int mem_pointer = 0;
  unsigned int bytes_sent = 0;

  do {
    bytes_sent += (512/8);
    tmp.data = mem_response[mem_pointer++];
    tmp.last = (bytes_sent >= header_length);
    tmp.keep = -1; // TODO
    http_response_headers.write(tmp);
  } while (bytes_sent < header_length);

  bytes_sent = 0;
  do {
    bytes_sent += (512/8);
    tmp.data = mem_response[mem_pointer++];
    tmp.last = (bytes_sent >= body_length);
    tmp.keep = -1; // TODO
    http_response_body.write(tmp);
  } while (bytes_sent < body_length);
}

void process_response(
  hls::stream<http::http_response_spt>& http_response,
  hls::stream<http::pkt512>& http_response_headers,
  hls::stream<http::pkt512>& http_response_body,
  hls::stream<internal_pkt>& strm_internal,
  ap_uint<512>* mem_response,
  unsigned int header_length,
  unsigned int body_length
) {
  enum class fsm_state { IDLE, NOTIFY, RESPOND };

  static fsm_state state = fsm_state::IDLE;
  static bool next_service_running = true;
  static http::http_meta meta;

  bool service_running = true;

  do {
    if (!strm_internal.empty()) {
      internal_pkt internal = strm_internal.read();
      
      write_tx_notify(
        http_response,
        internal.meta,
        header_length,
        body_length
      );

      write_tx_data(
        http_response_headers,
        http_response_body,
        mem_response,
        header_length,
        body_length
      );

      service_running = internal.service_running;
    }

  } while (service_running);
}

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
  unsigned int body_length
) {
#pragma HLS INTERFACE axis port=http_request
#pragma HLS INTERFACE axis port=http_request_headers
#pragma HLS INTERFACE axis port=http_request_body
#pragma HLS INTERFACE axis port=http_response
#pragma HLS INTERFACE axis port=http_response_headers
#pragma HLS INTERFACE axis port=http_response_body

#pragma HLS dataflow

  static hls::stream<internal_pkt> strm_internal("strm_internal");

  process_request(
    http_request,
    http_request_headers,
    http_request_body,
    strm_internal);

  process_response(
    http_response,
    http_response_headers,
    http_response_body,
    strm_internal,
    mem_response,
    header_length,
    body_length
  );
}