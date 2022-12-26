/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <iostream>
#include <string>
#include "http.h"
#include "http_lib.h"

using namespace http;

#define TEST_SIZE 8
int main (int argc, char* argv[]) {
  //
  // top-level streams
  //

  hls::stream<pkt16> tcp_listen_req("tcp_listen_req");
  hls::stream<pkt8> tcp_listen_rsp("tcp_listen_rsp");
  hls::stream<pkt128> tcp_notification("tcp_notification");
  hls::stream<pkt32> tcp_read_request("tcp_read_request");
  hls::stream<pkt16> tcp_rx_meta("tcp_rx_meta");
  hls::stream<pkt512> tcp_rx_data("tcp_rx_data");
  hls::stream<pkt32> tcp_tx_meta("tcp_tx_meta");
  hls::stream<pkt512> tcp_tx_data("tcp_tx_data");
  hls::stream<pkt64> tcp_tx_status("tcp_tx_status");
  hls::stream<http_request_spt> http_request("http_request");
  hls::stream<axi_stream_ispt> http_request_headers("http_request_headers");
  hls::stream<axi_stream_ispt> http_request_body("http_request_body");
  hls::stream<http_response_spt> http_response("http_response");
  hls::stream<axi_stream_ispt> http_response_headers("http_response_headers");
  hls::stream<axi_stream_ispt> http_response_body("http_response_body");

  //
  // mock data
  //

  const unsigned int mockSessionID = 12345;

  const std::string mockMethod[TEST_SIZE] = {
    "CONNECT / HTTP/2",
    "DELETE / HTTP/2",
    "GET / HTTP/2",
    "HEAD / HTTP/2",
    "OPTIONS / HTTP/2",
    "POST / HTTP/2",
    "PUT / HTTP/2",
    "WRONG"
  };

  HttpMethod expectedMethod[TEST_SIZE] = {
    HttpMethod::CONNECT,
    HttpMethod::DELETE,
    HttpMethod::GET,
    HttpMethod::HEAD,
    HttpMethod::OPTIONS,
    HttpMethod::POST,
    HttpMethod::PUT,
    HttpMethod::err
  };

  const std::string mockRequest = "Host: testnewsite.com\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:106.0) Gecko/20100101 Firefox/106.0\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\n"
    "Accept-Language: en-GB,en;q=0.5\n"
    "Accept-Encoding: gzip, deflate, br\n"
    "DNT: 1\n"
    "Upgrade-Insecure-Requests: 1\n"
    "Connection: keep-alive\n"
    "Sec-Fetch-Dest: document\n"
    "Sec-Fetch-Mode: navigate\n"
    "Sec-Fetch-Site: none\n"
    "Sec-Fetch-User: ?1\n"
    "Pragma: no-cache\n"
    "Cache-Control: no-cache\n\n";

  HttpStatus mockStatuses[TEST_SIZE] = {
    HttpStatus::OK_200,
    HttpStatus::NO_CONTENT_204,
    HttpStatus::MOVED_PERMANENTLY_301,
    HttpStatus::BAD_REQUEST_400,
    HttpStatus::FORBIDDEN_403,
    HttpStatus::NOT_FOUND_404,
    HttpStatus::INTERNAL_SERVER_ERROR_500,
    HttpStatus::NOT_IMPLEMENTED_501
  };

  const std::string expectedStatus[TEST_SIZE] = {
    "HTTP/1.1 200 OK",
    "HTTP/1.1 204 No Content",
    "HTTP/1.1 301 Moved Permanently",
    "HTTP/1.1 400 Bad Request"
    "HTTP/1.1 403 Forbidden",
    "HTTP/1.1 404 Not Found",
    "HTTP/1.1 500 Internal Server Error",
    "HTTP/1.1 501 Not Implemented"
  };

  const std::string mockHeaders = "Date: Thu, 10 Nov 2022 13:44:13 GMT\n"
    "Content-Type: text/html; charset=iso-8859-1\n"
    "Content-Length: 199\n"
    "Server: Apache\n"
    "x-iplb-request-id: 8184CA9B:8D0A_D5BA2132:0050_636D002D_8053:1CC34\n"
    "x-iplb-instance: 27847\n"
    "X-Cache: MISS from proxybd.ethz.ch\n"
    "Via: 1.1 proxybd.ethz.ch (squid/3.3.9)\n"
    "Connection: close\n";
  
  const std::string mockBody = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
    "<html><head>\n"
    "<title>403 Forbidden</title>\n"
    "</head><body>\n"
    "<h1>Forbidden</h1>\n"
    "<p>You don't have permission to access this resource.</p>\n"
    "</body></html>\n";

  //
  // execution
  //

  bool global_test = true;
  for (int i=0; i<TEST_SIZE; i++) {    
    bool this_test = true;
    bool last = false;

    // REQUEST flow

    tcp_notification_pkt notif;
    notif.sessionID = mockSessionID;
    notif.length = 5;
    notif.ipAddress = 1234;
    notif.dstPort = 8080;
    notif.closed = 0;
    tcp_notification.write(notif.serialise());

    pkt16 rx_meta;
    tcp_rx_meta.write(rx_meta);

    pkt512 rx_data;
    for (int j=0; j<mockMethod[i].size(); j++) {
      rx_data.data((j+1)*8-1, j*8) = mockMethod[i].at(j);
    }
    rx_data.keep = -1;
    rx_data.last = false;
    tcp_rx_data.write(rx_data);

    const unsigned int payload_size = mockRequest.size();
    for (int j=0; j<payload_size; j++) {
      auto jline = j % 64;
      rx_data.data((jline+1)*8-1, jline*8) = mockRequest.at(j);

      if ((((j+1) % 64 == 0)) || (j == payload_size - 1)) {
        rx_data.last = (j == payload_size - 1);
        tcp_rx_data.write(rx_data);
      }
    }

    do {
      pkt8 listen_rsp_pkt;
      tcp_listen_rsp.write(listen_rsp_pkt);

      http_top(
        tcp_listen_req,
        tcp_listen_rsp,
        tcp_notification,
        tcp_read_request,
        tcp_rx_meta,
        tcp_rx_data,
        tcp_tx_meta,
        tcp_tx_data,
        tcp_tx_status,
        http_request,
        http_request_headers,
        http_request_body,
        http_response,
        http_response_headers,
        http_response_body
      );

      if (!tcp_listen_req.empty()) {
        tcp_listen_req.read();
      }

      if (!http_request_body.empty()) {
        auto raw = http_request_body.read();
        last = raw.last;
      }
    } while (!last);

    // RESPONSE flow

    http_response_spt http_response_pkt;
    http_response_pkt.meta.sessionID = mockSessionID;
    http_response_pkt.meta.method = HttpMethod::GET;
    http_response_pkt.status_code = mockStatuses[i];
    http_response_pkt.body_size = mockBody.size();
    http_response_pkt.headers_size = mockHeaders.size();
    http_response.write(http_response_pkt);

    axi_stream_ispt tx_data;
    tx_data.last = false;
    tx_data.keep = -1;

    const unsigned int headers_size = mockHeaders.size();
    for (int j=0; j<headers_size; j++) {
      auto jline = j % 64;
      tx_data.data((jline+1)*8-1, jline*8) = mockHeaders.at(j);
      if ((((j+1) % 64 == 0)) || (j == headers_size - 1)) {
        tx_data.last = (j == headers_size - 1);
        http_response_headers.write(tx_data);
      }
    }

    const unsigned int body_size = mockBody.size();
    for (int j=0; j<body_size; j++) {
      auto jline = j % 64;
      tx_data.data((jline+1)*8-1, jline*8) = mockBody.at(j);
      if ((((j+1) % 64 == 0)) || (j == body_size - 1)) {
        tx_data.last = (j == body_size - 1);
        http_response_body.write(tx_data);
      }
    }

    tcp_tx_status_pkt tx_status;
    tx_status.sessionID = mockSessionID;
    tx_status.length = mockBody.size() + mockHeaders.size();
    tx_status.remaining_space = -1;
    tx_status.error = 0;
    tcp_tx_status.write(tx_status.serialise());

    last = false;
    do {
      pkt8 listen_rsp_pkt;
      tcp_listen_rsp.write(listen_rsp_pkt);

      http_top(
        tcp_listen_req,
        tcp_listen_rsp,
        tcp_notification,
        tcp_read_request,
        tcp_rx_meta,
        tcp_rx_data,
        tcp_tx_meta,
        tcp_tx_data,
        tcp_tx_status,
        http_request,
        http_request_headers,
        http_request_body,
        http_response,
        http_response_headers,
        http_response_body
      );

      if (!tcp_listen_req.empty()) {
        tcp_listen_req.read();
      }
      
      if (!tcp_tx_data.empty()) {
        auto raw = tcp_tx_data.read();
        last = raw.last;
      }
    } while (!last);

    //
    // assert
    //

    tcp_rxtx_request_pkt read_request = tcp_read_request.read();
    if (read_request.sessionID != notif.sessionID) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][read_request.sessionID]" << std::endl;
    }
    if (read_request.length != notif.length) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][read_request.length]" << std::endl;
    }
    auto request = http_request.read();
    if (request.meta.method != expectedMethod[i]) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.meta.method]" << std::endl;
    }
    if (request.meta.sessionID != notif.sessionID) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.meta.sessionID]" << std::endl;
    }
    if (request.method != expectedMethod[i]) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.method]" << std::endl;
    }
    while (!http_request_headers.empty()) {
      http_request_headers.read();
    }

    tcp_rxtx_request_pkt tx_meta = tcp_tx_meta.read();
    if (tx_meta.sessionID != mockSessionID) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][tcp_tx_meta.sessionID]" << std::endl;
    }
    if (tx_meta.length != (mockBody.size() + mockHeaders.size())) {
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
