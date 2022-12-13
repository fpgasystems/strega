/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <ap_axi_sdata.h>

namespace http {

typedef ap_axiu<512, 0, 0, 0> pkt512;
typedef ap_axiu<256, 0, 0, 0> pkt256;
typedef ap_axiu<128, 0, 0, 0> pkt128;
typedef ap_axiu<64, 0, 0, 0> pkt64;
typedef ap_axiu<32, 0, 0, 0> pkt32;
typedef ap_axiu<16, 0, 0, 0> pkt16;
typedef ap_axiu<8, 0, 0, 0> pkt8;

struct tcp_notification_pkt {
  ap_uint<16> sessionID;
  ap_uint<16> length;
  ap_uint<32> ipAddress;
  ap_uint<16> dstPort;
  ap_uint<1> closed;
  tcp_notification_pkt() {}
  tcp_notification_pkt(pkt128 raw)
    :sessionID(raw.data(15,0)),
    length(raw.data(31,16)),
    ipAddress(raw.data(63,32)),
    dstPort(raw.data(79,64)),
    closed(raw.data(80,80))
    { }
  pkt128 serialise() {
    pkt128 val;
    val.data(15,0) = sessionID;
    val.data(31,16) = length;
    val.data(63,32) = ipAddress;
    val.data(79,64) = dstPort;
    val.data(80,80) = closed;
    return val;
  }
};

struct tcp_rxtx_request_pkt {
  ap_uint<16> sessionID;
  ap_uint<16> length;
  tcp_rxtx_request_pkt() {}
  tcp_rxtx_request_pkt(pkt32 raw)
    :sessionID(raw.data(15,0)),
    length(raw.data(31,16))
    { }
  pkt32 serialise() {
    pkt32 val;
    val.data(15,0) = sessionID;
    val.data(31,16) = length;
    return val;
  }
};

struct tcp_tx_status_pkt {
  ap_uint<16> sessionID;
  ap_uint<16> length;
  ap_uint<32> remaining_space;
  ap_uint<2> error;
  tcp_tx_status_pkt() {}
  tcp_tx_status_pkt(pkt64 raw)
    :sessionID(raw.data(15,0)),
     length(raw.data(31,16)),
     remaining_space(raw.data(61,32)),
     error(raw.data(63,62))
     { }
  pkt64 serialise() {
    pkt64 val;
    val.data(15,0) = sessionID;
    val.data(31,16) = length;
    val.data(61,32) = remaining_space;
    val.data(63,62) = error;
    return val;
  }
};

} // namespace http