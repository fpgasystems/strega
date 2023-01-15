/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <string>
#include "../hls/http.h"

namespace http {

void listen_port_handler (
  hls::stream<ap_uint<16>>& tcp_listen_req,
  hls::stream<bool>& tcp_listen_rsp
) {
  #pragma HLS PIPELINE II=1

  ap_uint<16> tmp;
  tcp_listen_req.read(tmp);
  bool result = (tmp == 80);
  tcp_listen_rsp.write(result);
}

enum class fsm_state_rx {
  IDLE,
  NOTIFICATION,
  META,
  META_HANDSHAKE,
  DATA_HEADLINE,
  DATA_PAYLOAD,
  DATA_PAYLOAD2
};

enum class fsm_state_tx {
  IDLE,
  HANDSHAKE,
  CONSUME_DATA
};

const char MOCK_HEADLINE[] = "GET / HTTP/1.1\n";
const unsigned int MOCK_HEADLINE_SIZE = 18;

void rx_mock (
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_xx_req_pkt>& tcp_rx_req,
  hls::stream<ap_uint<16>>& tcp_rx_rsp,
  hls::stream<pkt512>& tcp_rx_data
) {
  #pragma HLS PIPELINE II=1

  static fsm_state_rx state = fsm_state_rx::IDLE;
  #pragma HLS reset variable=state

  switch (state) {
    case fsm_state_rx::IDLE: {
      state = fsm_state_rx::NOTIFICATION;
      break;
    }

    case fsm_state_rx::NOTIFICATION: {
          
      tcp_notification_pkt notif;
      notif.sessionID = 100;
      notif.length = 50;
      notif.ipAddress = 10;
      notif.dstPort = 10;
      notif.closed = false;
      tcp_notification.write(notif);

      state = fsm_state_rx::META;
      break;
    }

    case fsm_state_rx::META:
    {
      if (!tcp_rx_req.empty()) {
        tcp_rx_req.read();
        state = fsm_state_rx::META_HANDSHAKE;
      }
      break;
    }

    case fsm_state_rx::META_HANDSHAKE:
    {
      tcp_rx_rsp.write(static_cast<unsigned int>(state));
      state = fsm_state_rx::DATA_HEADLINE;
      break;
    }

    case fsm_state_rx::DATA_HEADLINE:
    {
      pkt512 raw;
      raw.last = false;
      for (int i=0; i < MOCK_HEADLINE_SIZE; ++i) {
        #pragma HLS UNROLL
        raw.data(i*8+7,i*8) = MOCK_HEADLINE[i];
      }
      tcp_rx_data.write(raw);
      
      state = fsm_state_rx::DATA_PAYLOAD;
      break;
    }

    case fsm_state_rx::DATA_PAYLOAD:
    {
      pkt512 raw;
      raw.last = false;
      raw.data = -1;
      tcp_rx_data.write(raw);

      state = fsm_state_rx::DATA_PAYLOAD2;
      break;
    }

    case fsm_state_rx::DATA_PAYLOAD2:
    {
      pkt512 raw;
      raw.last = true;
      raw.data = -1;
      tcp_rx_data.write(raw);

      state = fsm_state_rx::IDLE;
      break;
    }
  }
}

void tx_mock (
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<pkt512>& tcp_tx_data
)
{
  #pragma HLS PIPELINE II=1

  static fsm_state_tx state = fsm_state_tx::IDLE;
  #pragma HLS reset variable=state

  switch (state) {
    case fsm_state_tx::IDLE: {
      tcp_tx_req.read();
      state = fsm_state_tx::HANDSHAKE;
      break;
    }

    case fsm_state_tx::HANDSHAKE: {
      tcp_tx_rsp_pkt rsp;
      rsp.sessionID = 100;
      rsp.length = 10;
      rsp.remaining_space = -1;
      rsp.error = 0;
      tcp_tx_rsp.write(rsp);

      state = fsm_state_tx::CONSUME_DATA;
      break;
    }

    case fsm_state_tx::CONSUME_DATA: {
      pkt512 aux = tcp_tx_data.read();
      if (aux.last) {
        state = fsm_state_tx::IDLE;
      }
      break;
    }
  }
}

void tcp_mock (
  hls::stream<ap_uint<16>>& tcp_listen_req,
  hls::stream<bool>& tcp_listen_rsp,
  hls::stream<tcp_notification_pkt>& tcp_notification,
  hls::stream<tcp_xx_req_pkt>& tcp_rx_req,
  hls::stream<ap_uint<16>>& tcp_rx_rsp,
  hls::stream<pkt512>& tcp_rx_data,
  hls::stream<tcp_xx_req_pkt>& tcp_tx_req,
  hls::stream<tcp_tx_rsp_pkt>& tcp_tx_rsp,
  hls::stream<pkt512>& tcp_tx_data
) {
#pragma HLS INTERFACE axis port=tcp_listen_req
#pragma HLS INTERFACE axis port=tcp_listen_rsp
#pragma HLS INTERFACE axis port=tcp_notification
#pragma HLS INTERFACE axis port=tcp_rx_req
#pragma HLS INTERFACE axis port=tcp_rx_rsp
#pragma HLS INTERFACE axis port=tcp_rx_data
#pragma HLS INTERFACE axis port=tcp_tx_req
#pragma HLS INTERFACE axis port=tcp_tx_rsp
#pragma HLS INTERFACE axis port=tcp_tx_data

#pragma HLS aggregate variable=tcp_listen_req compact=bit
#pragma HLS aggregate variable=tcp_listen_rsp compact=bit
#pragma HLS aggregate variable=tcp_notification compact=bit
#pragma HLS aggregate variable=tcp_rx_req compact=bit
#pragma HLS aggregate variable=tcp_rx_rsp compact=bit
#pragma HLS aggregate variable=tcp_tx_req compact=bit
#pragma HLS aggregate variable=tcp_tx_rsp compact=bit

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW disable_start_propagation

  listen_port_handler(
    tcp_listen_req,
    tcp_listen_rsp
  );

  rx_mock(
    tcp_notification,
    tcp_rx_req,
    tcp_rx_rsp,
    tcp_rx_data
  );
  
  tx_mock(
    tcp_tx_req,
    tcp_tx_rsp,
    tcp_tx_data
  );
}

} // namespace http