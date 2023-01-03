/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

`default_nettype none
`timescale 1 ns / 1 ps

module http_core #(
  parameter integer C_TCP_LISTEN_REQ_TDATA_WIDTH          = 16                        ,
  parameter integer C_TCP_LISTEN_RSP_TDATA_WIDTH          = 8                         ,
  parameter integer C_TCP_NOTIFICATION_TDATA_WIDTH        = 128                       ,
  parameter integer C_TCP_RX_REQ_TDATA_WIDTH              = 32                        ,
  parameter integer C_TCP_RX_RSP_TDATA_WIDTH              = 16                        ,
  parameter integer C_TCP_RX_DATA_TDATA_WIDTH             = 512                       ,
  parameter integer C_TCP_TX_REQ_TDATA_WIDTH              = 32                        ,
  parameter integer C_TCP_TX_RSP_TDATA_WIDTH              = 64                        ,
  parameter integer C_TCP_TX_DATA_TDATA_WIDTH             = 512                       ,
  parameter integer C_HTTP_REQUEST_TDATA_WIDTH            = 256                       ,
  parameter integer C_HTTP_REQUEST_HEADERS_TDATA_WIDTH    = 512                       ,
  parameter integer C_HTTP_REQUEST_BODY_TDATA_WIDTH       = 512                       ,
  parameter integer C_HTTP_RESPONSE_TDATA_WIDTH           = 256                       ,
  parameter integer C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH   = 512                       ,
  parameter integer C_HTTP_RESPONSE_BODY_TDATA_WIDTH      = 512
)
(
  input  wire                                             ap_clk                      ,
  input  wire                                             ap_rst_n                    ,

  //
  // TCP-IP
  //
  
  output wire                                             tcp_listen_req_tvalid       ,
  input  wire                                             tcp_listen_req_tready       ,
  output wire [C_TCP_LISTEN_REQ_TDATA_WIDTH-1:0]          tcp_listen_req_tdata        ,
  
  input  wire                                             tcp_listen_rsp_tvalid       ,
  output wire                                             tcp_listen_rsp_tready       ,
  input  wire [C_TCP_LISTEN_RSP_TDATA_WIDTH-1:0]          tcp_listen_rsp_tdata        ,
  
  input  wire                                             tcp_notification_tvalid     ,
  output wire                                             tcp_notification_tready     ,
  input  wire [C_TCP_NOTIFICATION_TDATA_WIDTH-1:0]        tcp_notification_tdata      ,
  
  output wire                                             tcp_rx_req_tvalid           ,
  input  wire                                             tcp_rx_req_tready           ,
  output wire [C_TCP_RX_REQ_TDATA_WIDTH-1:0]              tcp_rx_req_tdata            ,
  
  input  wire                                             tcp_rx_rsp_tvalid           ,
  output wire                                             tcp_rx_rsp_tready           ,
  input  wire [C_TCP_RX_RSP_TDATA_WIDTH-1:0]              tcp_rx_rsp_tdata            ,
  
  input  wire                                             tcp_rx_data_tvalid          ,
  output wire                                             tcp_rx_data_tready          ,
  input  wire [C_TCP_RX_DATA_TDATA_WIDTH-1:0]             tcp_rx_data_tdata           ,
  input  wire [C_TCP_RX_DATA_TDATA_WIDTH/8-1:0]           tcp_rx_data_tkeep           ,
  input  wire                                             tcp_rx_data_tlast           ,
  
  output wire                                             tcp_tx_req_tvalid           ,
  input  wire                                             tcp_tx_req_tready           ,
  output wire [C_TCP_TX_REQ_TDATA_WIDTH-1:0]              tcp_tx_req_tdata            ,
  
  input  wire                                             tcp_tx_rsp_tvalid           ,
  output wire                                             tcp_tx_rsp_tready           ,
  input  wire [C_TCP_TX_RSP_TDATA_WIDTH-1:0]              tcp_tx_rsp_tdata            ,

  output wire                                             tcp_tx_data_tvalid          ,
  input  wire                                             tcp_tx_data_tready          ,
  output wire [C_TCP_TX_DATA_TDATA_WIDTH-1:0]             tcp_tx_data_tdata           ,
  output wire [C_TCP_TX_DATA_TDATA_WIDTH/8-1:0]           tcp_tx_data_tkeep           ,
  output wire                                             tcp_tx_data_tlast           ,

  //
  // APPLICATION
  //

  output wire                                             http_request_tvalid         ,
  input  wire                                             http_request_tready         ,
  output wire [C_HTTP_REQUEST_TDATA_WIDTH-1:0]            http_request_tdata          ,
  
  output wire                                             http_request_headers_tvalid ,
  input  wire                                             http_request_headers_tready ,
  output wire [C_HTTP_REQUEST_HEADERS_TDATA_WIDTH-1:0]    http_request_headers_tdata  ,
  output wire [C_HTTP_REQUEST_HEADERS_TDATA_WIDTH/8-1:0]  http_request_headers_tkeep  ,
  output wire                                             http_request_headers_tlast  ,
  
  output wire                                             http_request_body_tvalid    ,
  input  wire                                             http_request_body_tready    ,
  output wire [C_HTTP_REQUEST_BODY_TDATA_WIDTH-1:0]       http_request_body_tdata     ,
  output wire [C_HTTP_REQUEST_BODY_TDATA_WIDTH/8-1:0]     http_request_body_tkeep     ,
  output wire                                             http_request_body_tlast     ,
  
  input  wire                                             http_response_tvalid        ,
  output wire                                             http_response_tready        ,
  input  wire [C_HTTP_RESPONSE_TDATA_WIDTH-1:0]           http_response_tdata         ,
  
  input  wire                                             http_response_headers_tvalid,
  output wire                                             http_response_headers_tready,
  input  wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH-1:0]   http_response_headers_tdata ,
  input  wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH/8-1:0] http_response_headers_tkeep ,
  input  wire                                             http_response_headers_tlast ,
  
  input  wire                                             http_response_body_tvalid   ,
  output wire                                             http_response_body_tready   ,
  input  wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH-1:0]      http_response_body_tdata    ,
  input  wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH/8-1:0]    http_response_body_tkeep    ,
  input  wire                                             http_response_body_tlast
);

///////////////////////////////////////////////////////////////////////////////
// WIRES
///////////////////////////////////////////////////////////////////////////////

wire                                             slice_tcp_listen_req_tvalid;
wire                                             slice_tcp_listen_req_tready;
wire [C_TCP_LISTEN_REQ_TDATA_WIDTH-1:0]          slice_tcp_listen_req_tdata;

wire                                             slice_tcp_listen_rsp_tvalid;
wire                                             slice_tcp_listen_rsp_tready;
wire [C_TCP_LISTEN_RSP_TDATA_WIDTH-1:0]          slice_tcp_listen_rsp_tdata;

wire                                             slice_tcp_notification_tvalid;
wire                                             slice_tcp_notification_tready;
wire [C_TCP_NOTIFICATION_TDATA_WIDTH-1:0]        slice_tcp_notification_tdata;

wire                                             slice_tcp_rx_req_tvalid;
wire                                             slice_tcp_rx_req_tready;
wire [C_TCP_RX_REQ_TDATA_WIDTH-1:0]              slice_tcp_rx_req_tdata;

wire                                             slice_tcp_rx_rsp_tvalid;
wire                                             slice_tcp_rx_rsp_tready;
wire [C_TCP_RX_RSP_TDATA_WIDTH-1:0]              slice_tcp_rx_rsp_tdata;

wire                                             slice_tcp_rx_data_tvalid;
wire                                             slice_tcp_rx_data_tready;
wire [C_TCP_RX_DATA_TDATA_WIDTH-1:0]             slice_tcp_rx_data_tdata;
wire [C_TCP_RX_DATA_TDATA_WIDTH/8-1:0]           slice_tcp_rx_data_tkeep;
wire [C_TCP_RX_DATA_TDATA_WIDTH/8-1:0]           slice_tcp_rx_data_tstrb;
wire                                             slice_tcp_rx_data_tlast;

wire                                             slice_tcp_tx_req_tvalid;
wire                                             slice_tcp_tx_req_tready;
wire [C_TCP_TX_REQ_TDATA_WIDTH-1:0]              slice_tcp_tx_req_tdata;

wire                                             slice_tcp_tx_rsp_tvalid;
wire                                             slice_tcp_tx_rsp_tready;
wire [C_TCP_TX_RSP_TDATA_WIDTH-1:0]              slice_tcp_tx_rsp_tdata;

wire                                             slice_tcp_tx_data_tvalid;
wire                                             slice_tcp_tx_data_tready;
wire [C_TCP_TX_DATA_TDATA_WIDTH-1:0]             slice_tcp_tx_data_tdata;
wire [C_TCP_TX_DATA_TDATA_WIDTH/8-1:0]           slice_tcp_tx_data_tkeep;
wire [C_TCP_TX_DATA_TDATA_WIDTH/8-1:0]           slice_tcp_tx_data_tstrb;
wire                                             slice_tcp_tx_data_tlast;

wire                                             slice_http_request_tvalid;
wire                                             slice_http_request_tready;
wire [C_HTTP_REQUEST_TDATA_WIDTH-1:0]            slice_http_request_tdata;
  
wire                                             slice_http_request_headers_tvalid;
wire                                             slice_http_request_headers_tready;
wire [C_HTTP_REQUEST_HEADERS_TDATA_WIDTH-1:0]    slice_http_request_headers_tdata;
wire [C_HTTP_REQUEST_HEADERS_TDATA_WIDTH/8-1:0]  slice_http_request_headers_tkeep;
wire [C_HTTP_REQUEST_HEADERS_TDATA_WIDTH/8-1:0]  slice_http_request_headers_strb;
wire                                             slice_http_request_headers_tlast;
  
wire                                             slice_http_request_body_tvalid;
wire                                             slice_http_request_body_tready;
wire [C_HTTP_REQUEST_BODY_TDATA_WIDTH-1:0]       slice_http_request_body_tdata;
wire [C_HTTP_REQUEST_BODY_TDATA_WIDTH/8-1:0]     slice_http_request_body_tkeep;
wire [C_HTTP_REQUEST_BODY_TDATA_WIDTH/8-1:0]     slice_http_request_body_strb;
wire                                             slice_http_request_body_tlast;
  
wire                                             slice_http_response_tvalid;
wire                                             slice_http_response_tready;
wire [C_HTTP_RESPONSE_TDATA_WIDTH-1:0]           slice_http_response_tdata;

wire                                             slice_http_response_headers_tvalid;
wire                                             slice_http_response_headers_tready;
wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH-1:0]   slice_http_response_headers_tdata;
wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH/8-1:0] slice_http_response_headers_tkeep;
wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH/8-1:0] slice_http_response_headers_tstrb;
wire                                             slice_http_response_headers_tlast;
  
wire                                             slice_http_response_body_tvalid;
wire                                             slice_http_response_body_tready;
wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH-1:0]      slice_http_response_body_tdata;
wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH/8-1:0]    slice_http_response_body_tkeep;
wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH/8-1:0]    slice_http_response_body_tstrb;
wire                                             slice_http_response_body_tlast;

///////////////////////////////////////////////////////////////////////////////
// Combining weirdly generated signals from HLS
///////////////////////////////////////////////////////////////////////////////

assign slice_tcp_rx_data_tstrb = {C_TCP_RX_DATA_TDATA_WIDTH/8{1'b1}};
assign slice_http_response_headers_tstrb = {C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH/8{1'b1}};
assign slice_http_response_body_tstrb = {C_HTTP_RESPONSE_BODY_TDATA_WIDTH/8{1'b1}}

wire tcp_rx_data_V_data_V_TREADY;
wire tcp_rx_data_V_keep_V_TREADY;
wire tcp_rx_data_V_strb_V_TREADY;
wire tcp_rx_data_V_last_V_TREADY;
assign slice_tcp_rx_data_tready = tcp_rx_data_V_data_V_TREADY & tcp_rx_data_V_keep_V_TREADY & tcp_rx_data_V_last_V_TREADY;

wire tcp_tx_data_V_data_V_TVALID;
wire tcp_tx_data_V_keep_V_TVALID;
wire tcp_tx_data_V_strb_V_TVALID;
wire tcp_tx_data_V_last_V_TVALID;
assign slice_tcp_tx_data_tvalid = tcp_tx_data_V_data_V_TVALID & tcp_tx_data_V_keep_V_TVALID & tcp_tx_data_V_last_V_TVALID;

wire http_request_headers_V_data_V_TVALID;
wire http_request_headers_V_keep_V_TVALID;
wire http_request_headers_V_strb_V_TVALID;
wire http_request_headers_V_last_V_TVALID;
assign slice_http_request_headers_tvalid = http_request_headers_V_data_V_TVALID & http_request_headers_V_keep_V_TVALID & http_request_headers_V_last_V_TVALID;

wire http_request_body_V_data_V_TVALID;
wire http_request_body_V_keep_V_TVALID;
wire http_request_body_V_strb_V_TVALID;
wire http_request_body_V_last_V_TVALID;
assign slice_http_request_body_tvalid = http_request_body_V_data_V_TVALID & http_request_body_V_keep_V_TVALID & http_request_body_V_last_V_TVALID;

wire http_response_headers_V_data_V_TREADY;
wire http_response_headers_V_keep_V_TREADY;
wire http_response_headers_V_strb_V_TREADY;
wire http_response_headers_V_last_V_TREADY;
assign slice_http_response_headers_tready = http_response_headers_V_data_V_TREADY & http_response_headers_V_keep_V_TREADY & http_response_headers_V_last_V_TREADY;

wire http_response_body_V_data_V_TREADY;
wire http_response_body_V_keep_V_TREADY;
wire http_response_body_V_strb_V_TREADY;
wire http_response_body_V_last_V_TREADY;
assign slice_http_response_body_tready = http_response_body_V_data_V_TREADY & http_response_body_V_keep_V_TREADY & http_response_body_V_last_V_TREADY;

///////////////////////////////////////////////////////////////////////////////
// HTTP
///////////////////////////////////////////////////////////////////////////////

http_ip http_inst (
  .ap_clk(ap_clk),
  .ap_rst_n(ap_rst_n),

  //
  // TCP-IP
  //

  .tcp_listen_req_TDATA(slice_tcp_listen_req_tdata),
  .tcp_listen_req_TVALID(slice_tcp_listen_req_tvalid),
  .tcp_listen_req_TREADY(slice_tcp_listen_req_tready),

  .tcp_listen_rsp_TDATA(slice_tcp_listen_rsp_tdata),
  .tcp_listen_rsp_TVALID(slice_tcp_listen_rsp_tvalid),
  .tcp_listen_rsp_TREADY(slice_tcp_listen_rsp_tready),

  .tcp_notification_TDATA(slice_tcp_notification_tdata),
  .tcp_notification_TVALID(slice_tcp_notification_tvalid),
  .tcp_notification_TREADY(slice_tcp_notification_tready),

  .tcp_rx_req_TDATA(slice_tcp_rx_req_tdata),
  .tcp_rx_req_TVALID(slice_tcp_rx_req_tvalid),
  .tcp_rx_req_TREADY(slice_tcp_rx_req_tready),

  .tcp_rx_rsp_TDATA(slice_tcp_rx_rsp_tdata),
  .tcp_rx_rsp_TVALID(slice_tcp_rx_rsp_tvalid),
  .tcp_rx_rsp_TREADY(slice_tcp_rx_rsp_tready),

  .tcp_rx_data_V_data_V_TDATA(slice_tcp_rx_data_tdata),
  .tcp_rx_data_V_keep_V_TKEEP(slice_tcp_rx_data_tkeep),
  .tcp_rx_data_V_last_V_TLAST(slice_tcp_rx_data_tlast),
  .tcp_rx_data_V_strb_V_TSTRB(slice_tcp_rx_data_tstrb),
  .tcp_rx_data_V_data_V_TVALID(slice_tcp_rx_data_tvalid),
  .tcp_rx_data_V_data_V_TREADY(tcp_rx_data_V_data_V_TREADY),
  .tcp_rx_data_V_keep_V_TVALID(slice_tcp_rx_data_tvalid),
  .tcp_rx_data_V_keep_V_TREADY(tcp_rx_data_V_keep_V_TREADY),
  .tcp_rx_data_V_strb_V_TVALID(slice_tcp_rx_data_tvalid),
  .tcp_rx_data_V_strb_V_TREADY(tcp_rx_data_V_strb_V_TREADY),
  .tcp_rx_data_V_last_V_TVALID(slice_tcp_rx_data_tvalid),
  .tcp_rx_data_V_last_V_TREADY(tcp_rx_data_V_last_V_TREADY),

  .tcp_tx_req_TDATA(slice_tcp_tx_req_tdata),
  .tcp_tx_req_TVALID(slice_tcp_tx_req_tvalid),
  .tcp_tx_req_TREADY(slice_tcp_tx_req_tready),

  .tcp_tx_rsp_TDATA(slice_tcp_tx_rsp_tdata),
  .tcp_tx_rsp_TVALID(slice_tcp_tx_rsp_tvalid),
  .tcp_tx_rsp_TREADY(slice_tcp_tx_rsp_tready),

  .tcp_tx_data_V_data_V_TDATA(slice_tcp_tx_data_tdata),
  .tcp_tx_data_V_keep_V_TKEEP(slice_tcp_tx_data_tkeep),
  .tcp_tx_data_V_last_V_TLAST(slice_tcp_tx_data_tlast),
  .tcp_tx_data_V_strb_V_TSTRB(slice_tcp_tx_data_tstrb),
  .tcp_tx_data_V_data_V_TVALID(tcp_tx_data_V_data_V_TVALID),
  .tcp_tx_data_V_data_V_TREADY(slice_tcp_tx_data_tready),
  .tcp_tx_data_V_keep_V_TVALID(tcp_tx_data_V_keep_V_TVALID),
  .tcp_tx_data_V_keep_V_TREADY(slice_tcp_tx_data_tready),
  .tcp_tx_data_V_strb_V_TVALID(tcp_tx_data_V_strb_V_TVALID),
  .tcp_tx_data_V_strb_V_TREADY(slice_tcp_tx_data_tready),
  .tcp_tx_data_V_last_V_TVALID(tcp_tx_data_V_last_V_TVALID),
  .tcp_tx_data_V_last_V_TREADY(slice_tcp_tx_data_tready),

  //
  // APPLICATION
  //
  
  .http_request_TDATA(slice_http_request_tdata),
  .http_request_TVALID(slice_http_request_tvalid),
  .http_request_TREADY(slice_http_request_tready),

  .http_request_headers_V_data_V_TDATA(slice_http_request_headers_tdata),
  .http_request_headers_V_keep_V_TKEEP(slice_http_request_headers_tkeep),
  .http_request_headers_V_last_V_TLAST(slice_http_request_headers_tlast),
  .http_request_headers_V_strb_V_TSTRB(slice_http_request_headers_strb),
  .http_request_headers_V_data_V_TVALID(http_request_headers_V_data_V_TVALID),
  .http_request_headers_V_data_V_TREADY(slice_http_request_headers_tready),
  .http_request_headers_V_keep_V_TVALID(http_request_headers_V_keep_V_TVALID),
  .http_request_headers_V_keep_V_TREADY(slice_http_request_headers_tready),
  .http_request_headers_V_strb_V_TVALID(http_request_headers_V_strb_V_TVALID),
  .http_request_headers_V_strb_V_TREADY(slice_http_request_headers_tready),
  .http_request_headers_V_last_V_TVALID(http_request_headers_V_last_V_TVALID),
  .http_request_headers_V_last_V_TREADY(slice_http_request_headers_tready),

  .http_request_body_V_data_V_TDATA(slice_http_request_body_tdata),
  .http_request_body_V_keep_V_TKEEP(slice_http_request_body_tkeep),
  .http_request_body_V_last_V_TLAST(slice_http_request_body_tlast),
  .http_request_body_V_strb_V_TSTRB(slice_http_request_body_strb),
  .http_request_body_V_data_V_TVALID(http_request_body_V_data_V_TVALID),
  .http_request_body_V_data_V_TREADY(slice_http_request_body_tready),
  .http_request_body_V_keep_V_TVALID(http_request_body_V_keep_V_TVALID),
  .http_request_body_V_keep_V_TREADY(slice_http_request_body_tready),
  .http_request_body_V_strb_V_TVALID(http_request_body_V_strb_V_TVALID),
  .http_request_body_V_strb_V_TREADY(slice_http_request_body_tready),
  .http_request_body_V_last_V_TVALID(http_request_body_V_last_V_TVALID),
  .http_request_body_V_last_V_TREADY(slice_http_request_body_tready),

  .http_response_TDATA(slice_http_response_tdata),
  .http_response_TVALID(slice_http_response_tvalid),
  .http_response_TREADY(slice_http_response_tready),

  .http_response_headers_V_data_V_TDATA(slice_http_response_headers_tdata),
  .http_response_headers_V_keep_V_TKEEP(slice_http_response_headers_tkeep),
  .http_response_headers_V_last_V_TLAST(slice_http_response_headers_tlast),
  .http_response_headers_V_strb_V_TSTRB(slice_http_response_headers_tstrb),
  .http_response_headers_V_data_V_TVALID(slice_http_response_headers_tvalid),
  .http_response_headers_V_data_V_TREADY(http_response_headers_V_data_V_TREADY),
  .http_response_headers_V_keep_V_TVALID(slice_http_response_headers_tvalid),
  .http_response_headers_V_keep_V_TREADY(http_response_headers_V_keep_V_TREADY),
  .http_response_headers_V_strb_V_TVALID(slice_http_response_headers_tvalid),
  .http_response_headers_V_strb_V_TREADY(http_response_headers_V_strb_V_TREADY),
  .http_response_headers_V_last_V_TVALID(slice_http_response_headers_tvalid),
  .http_response_headers_V_last_V_TREADY(http_response_headers_V_last_V_TREADY),

  .http_response_body_V_data_V_TDATA(slice_http_response_body_tdata),
  .http_response_body_V_keep_V_TKEEP(slice_http_response_body_tkeep),
  .http_response_body_V_last_V_TLAST(slice_http_response_body_tlast),
  .http_response_body_V_strb_V_TSTRB(slice_http_response_body_tstrb),
  .http_response_body_V_data_V_TVALID(slice_http_response_body_tvalid),
  .http_response_body_V_data_V_TREADY(http_response_body_V_data_V_TREADY),
  .http_response_body_V_keep_V_TVALID(slice_http_response_body_tvalid),
  .http_response_body_V_keep_V_TREADY(http_response_body_V_keep_V_TREADY),
  .http_response_body_V_strb_V_TVALID(slice_http_response_body_tvalid),
  .http_response_body_V_strb_V_TREADY(http_response_body_V_strb_V_TREADY),
  .http_response_body_V_last_V_TVALID(slice_http_response_body_tvalid),
  .http_response_body_V_last_V_TREADY(http_response_body_V_last_V_TREADY)
);

///////////////////////////////////////////////////////////////////////////////
// Register slices to avoid combinatorial loops created by HLS due to the new
// axis INTERFACE (enforced since 19.1)
///////////////////////////////////////////////////////////////////////////////

axis_register_slice_16 tcp_listen_req_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_tcp_listen_req_tvalid),
  .s_axis_tready(slice_tcp_listen_req_tready),
  .s_axis_tdata(slice_tcp_listen_req_tdata),
  .m_axis_tvalid(tcp_listen_req_tvalid),
  .m_axis_tready(tcp_listen_req_tready),
  .m_axis_tdata(tcp_listen_req_tdata)
);

axis_register_slice_8 tcp_listen_rsp_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(tcp_listen_rsp_tvalid),
  .s_axis_tready(tcp_listen_rsp_tready),
  .s_axis_tdata(tcp_listen_rsp_tdata),
  .m_axis_tvalid(slice_tcp_listen_rsp_tvalid),
  .m_axis_tready(slice_tcp_listen_rsp_tready),
  .m_axis_tdata(slice_tcp_listen_rsp_tdata)
);

axis_register_slice_128 tcp_notification_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(tcp_notification_tvalid),
  .s_axis_tready(tcp_notification_tready),
  .s_axis_tdata(tcp_notification_tdata),
  .m_axis_tvalid(slice_tcp_notification_tvalid),
  .m_axis_tready(slice_tcp_notification_tready),
  .m_axis_tdata(slice_tcp_notification_tdata)
);

axis_register_slice_32 tcp_rx_req_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_tcp_rx_req_tvalid),
  .s_axis_tready(slice_tcp_rx_req_tready),
  .s_axis_tdata(slice_tcp_rx_req_tdata),
  .m_axis_tvalid(tcp_rx_req_tvalid),
  .m_axis_tready(tcp_rx_req_tready),
  .m_axis_tdata(tcp_rx_req_tdata)
);

axis_register_slice_16 tcp_rx_rsp_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(tcp_rx_rsp_tvalid),
  .s_axis_tready(tcp_rx_rsp_tready),
  .s_axis_tdata(tcp_rx_rsp_tdata),
  .m_axis_tvalid(slice_tcp_rx_rsp_tvalid),
  .m_axis_tready(slice_tcp_rx_rsp_tready),
  .m_axis_tdata(slice_tcp_rx_rsp_tdata)
);

axis_register_slice_512 tcp_rx_data_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(tcp_rx_data_tvalid),
  .s_axis_tready(tcp_rx_data_tready),
  .s_axis_tdata(tcp_rx_data_tdata),
  .s_axis_tkeep(tcp_rx_data_tkeep),
  .s_axis_tlast(tcp_rx_data_tlast),
  .m_axis_tvalid(slice_tcp_rx_data_tvalid),
  .m_axis_tready(slice_tcp_rx_data_tready),
  .m_axis_tdata(slice_tcp_rx_data_tdata),
  .m_axis_tkeep(slice_tcp_rx_data_tkeep),
  .m_axis_tlast(slice_tcp_rx_data_tlast)
);

axis_register_slice_32 tcp_tx_req_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_tcp_tx_req_tvalid),
  .s_axis_tready(slice_tcp_tx_req_tready),
  .s_axis_tdata(slice_tcp_tx_req_tdata),
  .m_axis_tvalid(tcp_tx_req_tvalid),
  .m_axis_tready(tcp_tx_req_tready),
  .m_axis_tdata(tcp_tx_req_tdata)
);

axis_register_slice_64 tcp_tx_rsp_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(tcp_tx_rsp_tvalid),
  .s_axis_tready(tcp_tx_rsp_tready),
  .s_axis_tdata(tcp_tx_rsp_tdata),
  .m_axis_tvalid(slice_tcp_tx_rsp_tvalid),
  .m_axis_tready(slice_tcp_tx_rsp_tready),
  .m_axis_tdata(slice_tcp_tx_rsp_tdata)
);

axis_register_slice_512 tcp_tx_data_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_tcp_tx_data_tvalid),
  .s_axis_tready(slice_tcp_tx_data_tready),
  .s_axis_tdata(slice_tcp_tx_data_tdata),
  .s_axis_tkeep(slice_tcp_tx_data_tkeep),
  .s_axis_tlast(slice_tcp_tx_data_tlast),
  .m_axis_tvalid(tcp_tx_data_tvalid),
  .m_axis_tready(tcp_tx_data_tready),
  .m_axis_tdata(tcp_tx_data_tdata),
  .m_axis_tkeep(tcp_tx_data_tkeep),
  .m_axis_tlast(tcp_tx_data_tlast)
);

axis_register_slice_256 http_request_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_http_request_tvalid),
  .s_axis_tready(slice_http_request_tready),
  .s_axis_tdata(slice_http_request_tdata),
  .m_axis_tvalid(http_request_tvalid),
  .m_axis_tready(http_request_tready),
  .m_axis_tdata(http_request_tdata)
);

axis_register_slice_512 http_request_headers_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_http_request_headers_tvalid),
  .s_axis_tready(slice_http_request_headers_tready),
  .s_axis_tdata(slice_http_request_headers_tdata),
  .s_axis_tkeep(slice_http_request_headers_tkeep),
  .s_axis_tlast(slice_http_request_headers_tlast),
  .m_axis_tvalid(http_request_headers_tvalid),
  .m_axis_tready(http_request_headers_tready),
  .m_axis_tdata(http_request_headers_tdata),
  .m_axis_tkeep(http_request_headers_tkeep),
  .m_axis_tlast(http_request_headers_tlast)
);

axis_register_slice_512 http_request_body_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(slice_http_request_body_tvalid),
  .s_axis_tready(slice_http_request_body_tready),
  .s_axis_tdata(slice_http_request_body_tdata),
  .s_axis_tkeep(slice_http_request_body_tkeep),
  .s_axis_tlast(slice_http_request_body_tlast),
  .m_axis_tvalid(http_request_body_tvalid),
  .m_axis_tready(http_request_body_tready),
  .m_axis_tdata(http_request_body_tdata),
  .m_axis_tkeep(http_request_body_tkeep),
  .m_axis_tlast(http_request_body_tlast)
);

axis_register_slice_256 http_response_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(http_response_tvalid),
  .s_axis_tready(http_response_tready),
  .s_axis_tdata(http_response_tdata),
  .m_axis_tvalid(slice_http_response_tvalid),
  .m_axis_tready(slice_http_response_tready),
  .m_axis_tdata(slice_http_response_tdata)
);

axis_register_slice_512 http_response_headers_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(http_response_headers_tvalid),
  .s_axis_tready(http_response_headers_tready),
  .s_axis_tdata(http_response_headers_tdata),
  .s_axis_tkeep(http_response_headers_tkeep),
  .s_axis_tlast(http_response_headers_tlast),
  .m_axis_tvalid(slice_http_response_headers_tvalid),
  .m_axis_tready(slice_http_response_headers_tready),
  .m_axis_tdata(slice_http_response_headers_tdata),
  .m_axis_tkeep(slice_http_response_headers_tkeep),
  .m_axis_tlast(slice_http_response_headers_tlast)
);

axis_register_slice_512 http_response_body_slice (
  .aclk(ap_clk),
  .aresetn(ap_rst_n),
  .s_axis_tvalid(http_response_body_tvalid),
  .s_axis_tready(http_response_body_tready),
  .s_axis_tdata(http_response_body_tdata),
  .s_axis_tkeep(http_response_body_tkeep),
  .s_axis_tlast(http_response_body_tlast),
  .m_axis_tvalid(slice_http_response_body_tvalid),
  .m_axis_tready(slice_http_response_body_tready),
  .m_axis_tdata(slice_http_response_body_tdata),
  .m_axis_tkeep(slice_http_response_body_tkeep),
  .m_axis_tlast(slice_http_response_body_tlast)
);

endmodule
`default_nettype wire