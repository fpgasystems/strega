/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

`timescale 1ns / 1ps

// `include "network_types.svh"
// `include "network_intf.svh"

module http #(parameter
  C_S_AXI_ADDR_WIDTH = 12,
  C_S_AXI_DATA_WIDTH = 32
)(
  input  wire                 ap_clk,
  input  wire                 ap_rst_n,
  // axi_lite.slave              axil_ctrl,

  axis_meta.slave             tcp_listen_req,
  axis_meta.master            tcp_listen_rsp,
  axis_meta.master            tcp_notification,
  axis_meta.slave             tcp_read_request,
  axis_meta.master            tcp_rx_meta,
  axis_meta.slave             tcp_tx_meta,
  axis_meta.master            tcp_tx_status,

  axi_stream.master           tcp_rx_data,
  axi_stream.slave            tcp_tx_data,

  axis_meta.slave             http_request,
  axi_stream.slave            http_request_headers,
  axi_stream.slave            http_request_body,
  axis_meta.master            http_response,
  axi_stream.master           http_response_headers,
  axi_stream.master           http_response_body
);

// logic interrupt;
// logic ap_start;
// logic ap_done;
// logic ap_idle;
// logic ap_ready;

assign tcp_listen_req.ready = 1'b0;
assign tcp_listen_rsp.valid = 1'b0;
assign tcp_notification.valid = 1'b0;
assign tcp_read_request.ready = 1'b0;
assign tcp_rx_meta.valid = 1'b0;
assign tcp_rx_data.valid = 1'b0;
assign tcp_tx_meta.ready = 1'b0;
assign tcp_tx_data.ready = 1'b0;
assign tcp_tx_status.valid = 1'b0;

assign http_request.ready = 1'b0;
assign http_request_headers.ready = 1'b0;
assign http_request_body.ready = 1'b0;
assign http_response.valid = 1'b0;
assign http_response_headers.valid = 1'b0;
assign http_response_body.valid = 1'b0;

http_ip http_inst (
  .ap_clk(ap_clk),
  .ap_rst_n(ap_rst_n),

  .tcp_listen_req_TDATA(tcp_listen_req.data),
  .tcp_listen_req_TVALID(tcp_listen_req.valid),
  .tcp_listen_req_TREADY(tcp_listen_req.ready),

  .tcp_listen_rsp_TDATA(tcp_listen_rsp.data),
  .tcp_listen_rsp_TVALID(tcp_listen_rsp.valid),
  .tcp_listen_rsp_TREADY(tcp_listen_rsp.ready),

  .tcp_notification_TDATA(tcp_notification.data),
  .tcp_notification_TVALID(tcp_notification.valid),
  .tcp_notification_TREADY(tcp_notification.ready),

  .tcp_read_request_TDATA(tcp_read_request.data),
  .tcp_read_request_TVALID(tcp_read_request.valid),
  .tcp_read_request_TREADY(tcp_read_request.ready),

  .tcp_rx_meta_TDATA(tcp_rx_meta.data),
  .tcp_rx_meta_TVALID(tcp_rx_meta.valid),
  .tcp_rx_meta_TREADY(tcp_rx_meta.ready),

  .tcp_tx_meta_TDATA(tcp_tx_meta.data),
  .tcp_tx_meta_TVALID(tcp_tx_meta.valid),
  .tcp_tx_meta_TREADY(tcp_tx_meta.ready),

  .tcp_tx_status_TDATA(tcp_tx_status.data),
  .tcp_tx_status_TVALID(tcp_tx_status.valid),
  .tcp_tx_status_TREADY(tcp_tx_status.ready),

  .tcp_rx_data_V_data_V_TDATA(tcp_rx_data.data),
  .tcp_rx_data_V_keep_V_TKEEP(tcp_rx_data.keep),
  .tcp_rx_data_V_last_V_TLAST(tcp_rx_data.last),
  .tcp_rx_data_V_strb_V_TSTRB(tcp_rx_data.keep),
  .tcp_rx_data_V_data_V_TVALID(tcp_rx_data.valid),
  .tcp_rx_data_V_data_V_TREADY(tcp_rx_data.ready),
  .tcp_rx_data_V_keep_V_TVALID(tcp_rx_data.valid),
  .tcp_rx_data_V_keep_V_TREADY(tcp_rx_data.ready),
  .tcp_rx_data_V_strb_V_TVALID(tcp_rx_data.valid),
  .tcp_rx_data_V_strb_V_TREADY(tcp_rx_data.ready),
  .tcp_rx_data_V_last_V_TVALID(tcp_rx_data.valid),
  .tcp_rx_data_V_last_V_TREADY(tcp_rx_data.ready),

  .tcp_tx_data_V_data_V_TDATA(tcp_tx_data.data),
  .tcp_tx_data_V_keep_V_TKEEP(tcp_tx_data.keep),
  .tcp_tx_data_V_last_V_TLAST(tcp_tx_data.last),
  .tcp_tx_data_V_strb_V_TSTRB(tcp_tx_data.keep),
  .tcp_tx_data_V_data_V_TVALID(tcp_tx_data.valid),
  .tcp_tx_data_V_data_V_TREADY(tcp_tx_data.ready),
  .tcp_tx_data_V_keep_V_TVALID(tcp_tx_data.valid),
  .tcp_tx_data_V_keep_V_TREADY(tcp_tx_data.ready),
  .tcp_tx_data_V_strb_V_TVALID(tcp_tx_data.valid),
  .tcp_tx_data_V_strb_V_TREADY(tcp_tx_data.ready),
  .tcp_tx_data_V_last_V_TVALID(tcp_tx_data.valid),
  .tcp_tx_data_V_last_V_TREADY(tcp_tx_data.ready),
  
  .http_request_TDATA(http_request.data),
  .http_request_TVALID(http_request.valid),
  .http_request_TREADY(http_request.ready),

  .http_request_headers_V_data_V_TDATA(http_request_headers.data),
  .http_request_headers_V_keep_V_TKEEP(http_request_headers.keep),
  .http_request_headers_V_last_V_TLAST(http_request_headers.last),
  .http_request_headers_V_strb_V_TSTRB(http_request_headers.keep),
  .http_request_headers_V_data_V_TVALID(http_request_headers.valid),
  .http_request_headers_V_data_V_TREADY(http_request_headers.ready),
  .http_request_headers_V_keep_V_TVALID(http_request_headers.valid),
  .http_request_headers_V_keep_V_TREADY(http_request_headers.ready),
  .http_request_headers_V_strb_V_TVALID(http_request_headers.valid),
  .http_request_headers_V_strb_V_TREADY(http_request_headers.ready),
  .http_request_headers_V_last_V_TVALID(http_request_headers.valid),
  .http_request_headers_V_last_V_TREADY(http_request_headers.ready),

  .http_request_body_V_data_V_TDATA(http_request_body.data),
  .http_request_body_V_keep_V_TKEEP(http_request_body.keep),
  .http_request_body_V_last_V_TLAST(http_request_body.last),
  .http_request_body_V_strb_V_TSTRB(http_request_body.keep),
  .http_request_body_V_data_V_TVALID(http_request_body.valid),
  .http_request_body_V_data_V_TREADY(http_request_body.ready),
  .http_request_body_V_keep_V_TVALID(http_request_body.valid),
  .http_request_body_V_keep_V_TREADY(http_request_body.ready),
  .http_request_body_V_strb_V_TVALID(http_request_body.valid),
  .http_request_body_V_strb_V_TREADY(http_request_body.ready),
  .http_request_body_V_last_V_TVALID(http_request_body.valid),
  .http_request_body_V_last_V_TREADY(http_request_body.ready),

  .http_response_TDATA(http_response.data),
  .http_response_TVALID(http_response.valid),
  .http_response_TREADY(http_response.ready),

  .http_response_headers_V_data_V_TDATA(http_response_headers.data),
  .http_response_headers_V_keep_V_TKEEP(http_response_headers.keep),
  .http_response_headers_V_last_V_TLAST(http_response_headers.last),
  .http_response_headers_V_strb_V_TSTRB(http_response_headers.keep),
  .http_response_headers_V_data_V_TVALID(http_response_headers.valid),
  .http_response_headers_V_data_V_TREADY(http_response_headers.ready),
  .http_response_headers_V_keep_V_TVALID(http_response_headers.valid),
  .http_response_headers_V_keep_V_TREADY(http_response_headers.ready),
  .http_response_headers_V_strb_V_TVALID(http_response_headers.valid),
  .http_response_headers_V_strb_V_TREADY(http_response_headers.ready),
  .http_response_headers_V_last_V_TVALID(http_response_headers.valid),
  .http_response_headers_V_last_V_TREADY(http_response_headers.ready),

  .http_response_body_V_data_V_TDATA(http_response_body.data),
  .http_response_body_V_keep_V_TKEEP(http_response_body.keep),
  .http_response_body_V_last_V_TLAST(http_response_body.last),
  .http_response_body_V_strb_V_TSTRB(http_response_body.keep),
  .http_response_body_V_data_V_TVALID(http_response_body.valid),
  .http_response_body_V_data_V_TREADY(http_response_body.ready),
  .http_response_body_V_keep_V_TVALID(http_response_body.valid),
  .http_response_body_V_keep_V_TREADY(http_response_body.ready),
  .http_response_body_V_strb_V_TVALID(http_response_body.valid),
  .http_response_body_V_strb_V_TREADY(http_response_body.ready),
  .http_response_body_V_last_V_TVALID(http_response_body.valid),
  .http_response_body_V_last_V_TREADY(http_response_body.ready)
);

// http_control_s_axi control_s_axi(
//   .ACLK(ap_clk),
//   .ARESET(~ap_rst_n),
//   .ACLK_EN(1'b1),
//   .AWADDR(axil_ctrl.awaddr),
//   .AWVALID(axil_ctrl.awvalid),
//   .AWREADY(axil_ctrl.awready),
//   .WDATA(axil_ctrl.wdata),
//   .WSTRB(axil_ctrl.wstrb),
//   .WVALID(axil_ctrl.wvalid),
//   .WREADY(axil_ctrl.wready),
//   .BRESP(axil_ctrl.bresp),
//   .BVALID(axil_ctrl.bvalid),
//   .BREADY(axil_ctrl.bready),
//   .ARADDR(axil_ctrl.araddr),
//   .ARVALID(axil_ctrl.arvalid),
//   .ARREADY(axil_ctrl.arready),
//   .RDATA(axil_ctrl.rdata),
//   .RRESP(axil_ctrl.rresp),
//   .RVALID(axil_ctrl.rvalid),
//   .RREADY(axil_ctrl.rready),
//   .interrupt(interrupt),
//   .ap_start(ap_start),
//   .ap_done(ap_done),
//   .ap_idl(ap_idle),
//   .ap_ready(ap_ready)
// );

endmodule