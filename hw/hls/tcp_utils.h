/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include "ap_axi_sdata.h"

namespace http {

typedef ap_axiu<512, 0, 0, 0> pkt512;

struct tcp_notification_pkt {
  ap_uint<16> sessionID;
  ap_uint<16> length;
  ap_uint<32> ipAddress;
  ap_uint<16> dstPort;
  bool closed;
  tcp_notification_pkt() {}
  // tcp_notification_pkt(pkt128 raw)
  //   :sessionID(raw.data(15,0)),
  //   length(raw.data(31,16)),
  //   ipAddress(raw.data(63,32)),
  //   dstPort(raw.data(79,64)),
  //   closed(raw.data(80,80))
  //   { }
  // pkt128 serialise() {
  //   pkt128 val;
  //   val.data(15,0) = sessionID;
  //   val.data(31,16) = length;
  //   val.data(63,32) = ipAddress;
  //   val.data(79,64) = dstPort;
  //   val.data(80,80) = closed;
  //   return val;
  // }
};

struct tcp_xx_req_pkt {
  ap_uint<16> sessionID;
  ap_uint<16> length;
  tcp_xx_req_pkt() {}
  // tcp_xx_req_pkt(pkt32 raw)
  //   :sessionID(raw.data(15,0)),
  //   length(raw.data(31,16))
  //   { }
  // pkt32 serialise() {
  //   pkt32 val;
  //   val.data(15,0) = sessionID;
  //   val.data(31,16) = length;
  //   return val;
  // }
};

struct tcp_tx_rsp_pkt {
  ap_uint<16> sessionID;
  ap_uint<16> length;
  ap_uint<30> remaining_space;
  ap_uint<2> error;
  tcp_tx_rsp_pkt() {}
  // tcp_tx_rsp_pkt(pkt64 raw)
  //   :sessionID(raw.data(15,0)),
  //    length(raw.data(31,16)),
  //    remaining_space(raw.data(61,32)),
  //    error(raw.data(63,62))
  //    { }
  // pkt64 serialise() {
  //   pkt64 val;
  //   val.data(15,0) = sessionID;
  //   val.data(31,16) = length;
  //   val.data(61,32) = remaining_space;
  //   val.data(63,62) = error;
  //   return val;
  // }
};

// internal AXI-stream

struct axi_stream_ispt {
  ap_uint<512> data;
  ap_uint<512/8> keep;
  bool last;
  axi_stream_ispt() {}
  axi_stream_ispt(pkt512 raw)
    :data(raw.data),
    keep(raw.keep),
    last(raw.last)
    { }
  pkt512 serialise() {
    pkt512 val;
    val.data = data;
    val.keep = keep;
    val.last = last;
    return val;
  }
};

} // namespace http