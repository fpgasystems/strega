/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <iostream>
#include <string>
#include "response_processor.h"
#include "http_lib.h"

using namespace http;

#define TEST_SIZE 2
int main (int argc, char* argv[]) {
  
  //
  // top-level streams
  //

  hls::stream<pkt32> tcp_tx_meta("tcp_tx_meta");
  hls::stream<pkt512> tcp_tx_data("tcp_tx_data");
  hls::stream<pkt64> tcp_tx_status("tcp_tx_status");
  hls::stream<http_response_spt> http_response("http_response");
  hls::stream<axi_stream_ispt> http_response_headers("http_response_headers");
  hls::stream<axi_stream_ispt> http_response_body("http_response_body");

  //
  // mock data
  //

  const unsigned int sessionID = 12345;

  const std::string output[TEST_SIZE] = {
    "HTTP/1.1 200 OK",
    "HTTP/1.1 403 Forbidden"
  };

  const std::string headers = "Date: Thu, 10 Nov 2022 13:44:13 GMT\n"
    "Content-Type: text/html; charset=iso-8859-1\n"
    "Content-Length: 199\n"
    "Server: Apache\n"
    "x-iplb-request-id: 8184CA9B:8D0A_D5BA2132:0050_636D002D_8053:1CC34\n"
    "x-iplb-instance: 27847\n"
    "X-Cache: MISS from proxybd.ethz.ch\n"
    "Via: 1.1 proxybd.ethz.ch (squid/3.3.9)\n"
    "Connection: close\n";
  
  const std::string body = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
    "<html><head>\n"
    "<title>403 Forbidden</title>\n"
    "</head><body>\n"
    "<h1>Forbidden</h1>\n"
    "<p>You don't have permission to access this resource.</p>\n"
    "</body></html>\n";
  
  HttpStatus statuses[TEST_SIZE] = {
    HttpStatus::OK_200,
    HttpStatus::FORBIDDEN_403
  };

  //
  // execution
  //

  bool global_test = true;
  for (int i=0; i<TEST_SIZE; i++) {
    bool this_test = true;

    http_response_spt http_response_pkt;
    http_response_pkt.meta.sessionID = sessionID;
    http_response_pkt.meta.method = HttpMethod::GET;
    http_response_pkt.status_code = statuses[i];
    http_response_pkt.body_size = body.size();
    http_response_pkt.headers_size = headers.size();
    http_response.write(http_response_pkt);

    axi_stream_ispt tx_data;
    tx_data.last = false;
    tx_data.keep = -1;

    const unsigned int headers_size = headers.size();
    for (int j=0; j<headers_size; j++) {
      auto jline = j % 64;
      tx_data.data((jline+1)*8-1, jline*8) = headers.at(j);
      if ((((j+1) % 64 == 0)) || (j == headers_size - 1)) {
        tx_data.last = (j == headers_size - 1);
        http_response_headers.write(tx_data);
      }
    }

    const unsigned int body_size = body.size();
    for (int j=0; j<body_size; j++) {
      auto jline = j % 64;
      tx_data.data((jline+1)*8-1, jline*8) = body.at(j);
      if ((((j+1) % 64 == 0)) || (j == body_size - 1)) {
        tx_data.last = (j == body_size - 1);
        http_response_body.write(tx_data);
      }
    }

    tcp_tx_status_pkt tx_status;
    tx_status.sessionID = sessionID;
    tx_status.length = body.size() + headers.size();
    tx_status.remaining_space = -1;
    tx_status.error = 0;
    tcp_tx_status.write(tx_status.serialise());

    bool last = false;
    do {
      response_processor(
        tcp_tx_meta,
        tcp_tx_data,
        tcp_tx_status,
        http_response,
        http_response_headers,
        http_response_body
      );

      if (!tcp_tx_data.empty()) {
        auto raw = tcp_tx_data.read();
        last = raw.last;
      }
    } while (!last);

    tcp_rxtx_request_pkt tx_meta = tcp_tx_meta.read();
    if (tx_meta.sessionID != sessionID) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][tcp_tx_meta.sessionID]" << std::endl;
    }
    if (tx_meta.length != (body.size() + headers.size())) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][tcp_tx_meta.length]" << std::endl;
    }

    if (this_test) {
      std::cout << "PASS: TEST[" << i << "]" << std::endl;
    }
    global_test &= this_test;
  }

  return !global_test;
}