/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <iostream>
#include <string>
#include "request_processor.h"
#include "http_lib.h"

using namespace http;

#define TEST_SIZE 8
int main (int argc, char* argv[]) {
  //
  // top-level streams
  //

  hls::stream<pkt128> tcp_notification("tcp_notification");
  hls::stream<pkt32> tcp_read_request("tcp_read_request");
  hls::stream<pkt16> tcp_rx_meta("tcp_rx_meta");
  hls::stream<pkt512> tcp_rx_data("tcp_rx_data");
  hls::stream<http_request_spt> http_request("http_request");
  hls::stream<axi_stream_ispt> http_request_headers("http_request_headers");
  hls::stream<axi_stream_ispt> http_request_body("http_request_body");

  //
  // mock data
  //

  const std::string input[TEST_SIZE] = {
    "CONNECT / HTTP/2",
    "DELETE / HTTP/2",
    "GET / HTTP/2",
    "HEAD / HTTP/2",
    "OPTIONS / HTTP/2",
    "POST / HTTP/2",
    "PUT / HTTP/2",
    "WRONG"
  };

  const std::string payload = "Host: testnewsite.com\n"
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
  
  HttpMethod output[TEST_SIZE] = {
    HttpMethod::CONNECT,
    HttpMethod::DELETE,
    HttpMethod::GET,
    HttpMethod::HEAD,
    HttpMethod::OPTIONS,
    HttpMethod::POST,
    HttpMethod::PUT,
    HttpMethod::err
  };

  //
  // execution
  //

  bool global_test = true;
  for (int i=0; i<TEST_SIZE; i++) {    
    bool this_test = true;

    tcp_notification_pkt notif;
    notif.sessionID = i;
    notif.length = 5;
    notif.ipAddress = 1234;
    notif.dstPort = 8080;
    notif.closed = 0;
    tcp_notification.write(notif.serialise());

    pkt16 rx_meta;
    tcp_rx_meta.write(rx_meta);

    pkt512 rx_data;
    for (int j=0; j<input[i].size(); j++) {
      rx_data.data((j+1)*8-1, j*8) = input[i].at(j);
    }
    rx_data.keep = -1;
    rx_data.last = false;
    tcp_rx_data.write(rx_data);

    const unsigned int payload_size = payload.size();
    for (int j=0; j<payload_size; j++) {
      auto jline = j % 64;
      rx_data.data((jline+1)*8-1, jline*8) = payload.at(j);
      // std::cout << payload.at(j);

      if ((((j+1) % 64 == 0)) || (j == payload_size - 1)) {
        // std::cout << std::endl << "*****" << std::endl;
        rx_data.last = (j == payload_size - 1);
        tcp_rx_data.write(rx_data);
      }
    }

    bool last = false;
    do {
      request_processor(
        tcp_notification,
        tcp_read_request,
        tcp_rx_meta,
        tcp_rx_data,
        http_request,
        http_request_headers,
        http_request_body
      );
      if (!http_request_body.empty()) {
        auto raw = http_request_body.read();
        // for (int i=0; i<512/8; i++) {
        //   std::cout << (char)raw.data(i*8+7, i*8);
        // }
        last = raw.last;
      }
    } while (!last);
    // std::cout << std::endl;

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
    if (request.meta.method != output[i]) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.meta.method]" << std::endl;
    }
    if (request.meta.sessionID != notif.sessionID) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.meta.sessionID]" << std::endl;
    }
    if (request.method != output[i]) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.method]" << std::endl;
    }
    // TODO
    /*if (request.endpoint != ) {
      this_test = false;
      std::cerr << "ERROR: [" << i << "][http_request.endpoint]" << std::endl;
    }*/

    // std::cout << "HEADERS=";
    while (!http_request_headers.empty()) {
      auto raw = http_request_headers.read();
      // for (int i=0; i<512/8; i++) {
      //   std::cout << (char)raw.data(i*8+7, i*8);
      // }
    }

    if (this_test) {
      std::cout << "PASS: TEST[" << i << "]" << std::endl;
    }
    global_test &= this_test;
  }

  return !global_test;
}
