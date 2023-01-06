/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <hls_stream.h>
#include <stdint.h>
#include <fstream>
#include <iomanip>
#include "ap_axi_sdata.h"

#include "ap_int.h"
#include "lynx_hls_c.hpp" // TODO: Adjust the vFPGA ids

#include "http.h"

void wrapper_coyote (
    // EN_STRM
    // Host streams
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> >& axis_host_sink,
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> >& axis_host_src,

    // EN_MEM
    // Card streams
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> >& axis_card_sink,
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> >& axis_card_src,

    // EN_TCP_0
    // TCP/IP descriptors
    hls::stream<tcpListenReqIntf>& tcp_0_listen_req,
    hls::stream<tcpListenRspIntf>& tcp_0_listen_rsp,
    hls::stream<tcpOpenReqIntf>& tcp_0_open_req,        // NOT USED
    hls::stream<tcpOpenRspIntf>& tcp_0_open_rsp,        // NOT USED
    hls::stream<tcpCloseReqIntf>& tcp_0_close_req,      // NOT USED
    hls::stream<tcpNotifyIntf>& tcp_0_notify,
    hls::stream<tcpRdPkgIntf>& tcp_0_rd_package,
    hls::stream<tcpRxMetaIntf>& tcp_0_rx_meta,
    hls::stream<tcpTxMetaIntf>& tcp_0_tx_meta,
    hls::stream<tcpTxStatIntf>& tcp_0_tx_stat,

    // TCP/IP streams
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> >& axis_tcp_0_sink,
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> >& axis_tcp_0_src,
    ap_uint<64> axi_ctrl
) {
    #pragma HLS DATAFLOW disable_start_propagation
    #pragma HLS INTERFACE ap_ctrl_none port=return  

    // EN_STRN
    #pragma HLS INTERFACE axis register port=axis_host_sink name=s_axis_host_sink
    #pragma HLS INTERFACE axis register port=axis_host_src name=m_axis_host_src

    // EN_MEM
    #pragma HLS INTERFACE axis register port=axis_card_sink name=s_axis_card_sink
    #pragma HLS INTERFACE axis register port=axis_card_src name=m_axis_card_src

    // EN_TCP_0
    #pragma HLS INTERFACE axis register port=tcp_0_listen_req name=m_tcp_0_listen_req
    #pragma HLS INTERFACE axis register port=tcp_0_listen_rsp name=s_tcp_0_listen_rsp
    #pragma HLS INTERFACE axis register port=tcp_0_open_req name=m_tcp_0_open_req
    #pragma HLS INTERFACE axis register port=tcp_0_open_rsp name=s_tcp_0_open_rsp
    #pragma HLS INTERFACE axis register port=tcp_0_close_req name=m_tcp_0_close_req
    #pragma HLS INTERFACE axis register port=tcp_0_notify name=s_tcp_0_notify
    #pragma HLS INTERFACE axis register port=tcp_0_rd_pkg name=m_tcp_0_rd_pkg
    #pragma HLS INTERFACE axis register port=tcp_0_rx_meta name=m_tcp_0_rx_meta
    #pragma HLS INTERFACE axis register port=tcp_0_tx_meta name=m_tcp_0_tx_meta
    #pragma HLS INTERFACE axis register port=tcp_0_tx_stat name=s_tcp_0_tx_stat
    #pragma HLS INTERFACE axis register port=axis_tcp_0_src name=m_axis_tcp_0_src
    #pragma HLS INTERFACE axis register port=axis_tcp_0_sink name=s_axis_tcp_0_sink
    #pragma HLS aggregate variable=tcp_0_listen_req compact=bit
    #pragma HLS aggregate variable=tcp_0_listen_rsp compact=bit
    #pragma HLS aggregate variable=tcp_0_open_req compact=bit
    #pragma HLS aggregate variable=tcp_0_open_rsp compact=bit
    #pragma HLS aggregate variable=tcp_0_close_req compact=bit
    #pragma HLS aggregate variable=tcp_0_notify compact=bit
    #pragma HLS aggregate variable=tcp_0_rd_pkg compact=bit
    #pragma HLS aggregate variable=tcp_0_rx_meta compact=bit
    #pragma HLS aggregate variable=tcp_0_tx_meta compact=bit
    #pragma HLS aggregate variable=tcp_0_tx_stat compact=bit
    
    #pragma HLS INTERFACE s_axilite port=return     bundle=control
    //#pragma HLS INTERFACE s_axilite port=axi_ctrl_a bundle=control
    //#pragma HLS INTERFACE s_axilite port=axi_ctrl_b bundle=control
    //#pragma HLS INTERFACE s_axilite port=axi_ctrl_c bundle=control


    // // EN_STRM
    // ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> tmp_axis_host_sink = axis_host_sink.read();
    // axis_host_src.write(ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0>());

    // // EN_MEM
    // ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> tmp_axis_card_sink = axis_card_sink.read();
    // axis_card_src.write(ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0>());

    // // EN_TCP_0
    // tcp_0_listen_req.write(tcpListenReqIntf());
    // tcpListenRspIntf tmp_tcp_0_listen_rsp = tcp_0_listen_rsp.read();
    // tcp_0_open_req.write(tcpOpenReqIntf());
    // tcpOpenRspIntf tmp_tcp_0_open_rsp = tcp_0_open_rsp.read();
    // tcp_0_close_req.write(tcpCloseReqIntf());
    // tcpNotifyIntf tmp_tcp_0_notify = tcp_0_notify.read();
    // tcp_0_rd_pkg.write(tcpRdPkgIntf());
    // tcp_0_rx_meta.write(tcpRxMetaIntf());
    // tcp_0_tx_meta.write(tcpTxMetaIntf());
    // tcpTxStatIntf tmp_tcp_0_tx_stat = tcp_0_tx_stat.read();
    // ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0> tmp_axis_tcp_0_sink = axis_tcp_0_sink.read();
    // axis_tcp_0_src.write(ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0>());

    http(
      // TCP-IP
      tcp_0_listen_req
      tcp_0_listen_rsp,
      tcp_0_notify,
      tcp_0_rd_package,
      tcp_0_rx_meta,
      hls::stream<pkt512>& tcp_rx_data,
      tcp_0_tx_meta,
      hls::stream<pkt512>& tcp_tx_data,
      tcp_0_tx_stat,
      // Application
      hls::stream<http_request_spt>& http_request,
      hls::stream<pkt512>& http_request_headers,
      hls::stream<pkt512>& http_request_body,
      hls::stream<http_response_spt>& http_response,
      hls::stream<pkt512>& http_response_headers,
      hls::stream<pkt512>& http_response_body
    );

} 