/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

`default_nettype none
`timescale 1 ns / 1 ps

module http #(
  parameter integer C_S_AXI_CONTROL_ADDR_WIDTH                = 12                    ,
  parameter integer C_S_AXI_CONTROL_DATA_WIDTH                = 32                    ,
  parameter integer C_UDP_RX_TDATA_WIDTH                      = 512                   ,
  parameter integer C_UDP_TX_TDATA_WIDTH                      = 512                   ,
  parameter integer C_UDP_RX_META_TDATA_WIDTH                 = 256                   ,
  parameter integer C_UDP_TX_META_TDATA_WIDTH                 = 256                   ,
  parameter integer C_TCP_LISTEN_PORT_TDATA_WIDTH             = 16                    ,
  parameter integer C_TCP_PORT_STATUS_TDATA_WIDTH             = 8                     ,
  parameter integer C_TCP_OPEN_CONN_TDATA_WIDTH               = 64                    ,
  parameter integer C_TCP_OPEN_STATUS_TDATA_WIDTH             = 128                   ,
  parameter integer C_TCP_CLOSE_CONN_TDATA_WIDTH              = 16                    ,
  parameter integer C_TCP_NOTIFICATION_TDATA_WIDTH            = 128                   ,
  parameter integer C_TCP_READ_PKG_TDATA_WIDTH                = 32                    ,
  parameter integer C_TCP_RX_META_TDATA_WIDTH                 = 16                    ,
  parameter integer C_TCP_RX_DATA_TDATA_WIDTH                 = 512                   ,
  parameter integer C_TCP_TX_META_TDATA_WIDTH                 = 32                    ,
  parameter integer C_TCP_TX_DATA_TDATA_WIDTH                 = 512                   ,
  parameter integer C_TCP_TX_STATUS_TDATA_WIDTH               = 64                    ,
  parameter integer C_HTTP_REQUEST_TDATA_WIDTH                = 256                   ,
  parameter integer C_HTTP_REQUEST_HEADERS_TDATA_WIDTH        = 512                   ,
  parameter integer C_HTTP_REQUEST_BODY_TDATA_WIDTH           = 512                   ,
  parameter integer C_HTTP_RESPONSE_TDATA_WIDTH               = 256                   ,
  parameter integer C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH       = 512                   ,
  parameter integer C_HTTP_RESPONSE_BODY_TDATA_WIDTH          = 512
)
(
  input  wire                                             ap_clk                      ,
  input  wire                                             ap_rst_n                    ,

  //
  // TCP-IP
  //

  input  wire                                             udp_rx_tvalid               ,
  output wire                                             udp_rx_tready               ,
  input  wire [C_UDP_RX_TDATA_WIDTH-1:0]                  udp_rx_tdata                ,
  input  wire [C_UDP_RX_TDATA_WIDTH/8-1:0]                udp_rx_tkeep                ,
  input  wire                                             udp_rx_tlast                ,

  output wire                                             udp_tx_tvalid               ,
  input  wire                                             udp_tx_tready               ,
  output wire [C_UDP_TX_TDATA_WIDTH-1:0]                  udp_tx_tdata                ,
  output wire [C_UDP_TX_TDATA_WIDTH/8-1:0]                udp_tx_tkeep                ,
  output wire                                             udp_tx_tlast                ,
  
  input  wire                                             udp_rx_meta_tvalid          ,
  output wire                                             udp_rx_meta_tready          ,
  input  wire [C_UDP_RX_META_TDATA_WIDTH-1:0]             udp_rx_meta_tdata           ,
  input  wire [C_UDP_RX_META_TDATA_WIDTH/8-1:0]           udp_rx_meta_tkeep           ,
  input  wire                                             udp_rx_meta_tlast           ,
  
  output wire                                             udp_tx_meta_tvalid          ,
  input  wire                                             udp_tx_meta_tready          ,
  output wire [C_UDP_TX_META_TDATA_WIDTH-1:0]             udp_tx_meta_tdata           ,
  output wire [C_UDP_TX_META_TDATA_WIDTH/8-1:0]           udp_tx_meta_tkeep           ,
  output wire                                             udp_tx_meta_tlast           ,
  
  output wire                                             tcp_listen_port_tvalid      ,
  input  wire                                             tcp_listen_port_tready      ,
  output wire [C_TCP_LISTEN_PORT_TDATA_WIDTH-1:0]         tcp_listen_port_tdata       ,
  output wire [C_TCP_LISTEN_PORT_TDATA_WIDTH/8-1:0]       tcp_listen_port_tkeep       ,
  output wire                                             tcp_listen_port_tlast       ,
  
  input  wire                                             tcp_port_status_tvalid      ,
  output wire                                             tcp_port_status_tready      ,
  input  wire [C_TCP_PORT_STATUS_TDATA_WIDTH-1:0]         tcp_port_status_tdata       ,
  input  wire                                             tcp_port_status_tlast       ,
  
  output wire                                             tcp_open_conn_tvalid        ,
  input  wire                                             tcp_open_conn_tready        ,
  output wire [C_TCP_OPEN_CONN_TDATA_WIDTH-1:0]           tcp_open_conn_tdata         ,
  output wire [C_TCP_OPEN_CONN_TDATA_WIDTH/8-1:0]         tcp_open_conn_tkeep         ,
  output wire                                             tcp_open_conn_tlast         ,
  
  input  wire                                             tcp_open_status_tvalid      ,
  output wire                                             tcp_open_status_tready      ,
  input  wire [C_TCP_OPEN_STATUS_TDATA_WIDTH-1:0]         tcp_open_status_tdata       ,
  input  wire [C_TCP_OPEN_STATUS_TDATA_WIDTH/8-1:0]       tcp_open_status_tkeep       ,
  input  wire                                             tcp_open_status_tlast       ,
  
  output wire                                             tcp_close_conn_tvalid       ,
  input  wire                                             tcp_close_conn_tready       ,
  output wire [C_TCP_CLOSE_CONN_TDATA_WIDTH-1:0]          tcp_close_conn_tdata        ,
  output wire [C_TCP_CLOSE_CONN_TDATA_WIDTH/8-1:0]        tcp_close_conn_tkeep        ,
  output wire                                             tcp_close_conn_tlast        ,
  
  input  wire                                             tcp_notification_tvalid     ,
  output wire                                             tcp_notification_tready     ,
  input  wire [C_TCP_NOTIFICATION_TDATA_WIDTH-1:0]        tcp_notification_tdata      ,
  input  wire [C_TCP_NOTIFICATION_TDATA_WIDTH/8-1:0]      tcp_notification_tkeep      ,
  input  wire                                             tcp_notification_tlast      ,
  
  output wire                                             tcp_read_pkg_tvalid         ,
  input  wire                                             tcp_read_pkg_tready         ,
  output wire [C_TCP_READ_PKG_TDATA_WIDTH-1:0]            tcp_read_pkg_tdata          ,
  output wire [C_TCP_READ_PKG_TDATA_WIDTH/8-1:0]          tcp_read_pkg_tkeep          ,
  output wire                                             tcp_read_pkg_tlast          ,
  
  input  wire                                             tcp_rx_meta_tvalid          ,
  output wire                                             tcp_rx_meta_tready          ,
  input  wire [C_TCP_RX_META_TDATA_WIDTH-1:0]             tcp_rx_meta_tdata           ,
  input  wire [C_TCP_RX_META_TDATA_WIDTH/8-1:0]           tcp_rx_meta_tkeep           ,
  input  wire                                             tcp_rx_meta_tlast           ,
  
  input  wire                                             tcp_rx_data_tvalid          ,
  output wire                                             tcp_rx_data_tready          ,
  input  wire [C_TCP_RX_DATA_TDATA_WIDTH-1:0]             tcp_rx_data_tdata           ,
  input  wire [C_TCP_RX_DATA_TDATA_WIDTH/8-1:0]           tcp_rx_data_tkeep           ,
  input  wire                                             tcp_rx_data_tlast           ,
  
  output wire                                             tcp_tx_meta_tvalid          ,
  input  wire                                             tcp_tx_meta_tready          ,
  output wire [C_TCP_TX_META_TDATA_WIDTH-1:0]             tcp_tx_meta_tdata           ,
  output wire [C_TCP_TX_META_TDATA_WIDTH/8-1:0]           tcp_tx_meta_tkeep           ,
  output wire                                             tcp_tx_meta_tlast           ,
  
  output wire                                             tcp_tx_data_tvalid          ,
  input  wire                                             tcp_tx_data_tready          ,
  output wire [C_TCP_TX_DATA_TDATA_WIDTH-1:0]             tcp_tx_data_tdata           ,
  output wire [C_TCP_TX_DATA_TDATA_WIDTH/8-1:0]           tcp_tx_data_tkeep           ,
  output wire                                             tcp_tx_data_tlast           ,
  
  input  wire                                             tcp_tx_status_tvalid        ,
  output wire                                             tcp_tx_status_tready        ,
  input  wire [C_TCP_TX_STATUS_TDATA_WIDTH-1:0]           tcp_tx_status_tdata         ,
  input  wire [C_TCP_TX_STATUS_TDATA_WIDTH/8-1:0]         tcp_tx_status_tkeep         ,
  input  wire                                             tcp_tx_status_tlast         ,

  //
  // APPLICATION
  //

  output wire                                             http_request_tvalid         ,
  input  wire                                             http_request_tready         ,
  output wire [C_HTTP_REQUEST_TDATA_WIDTH-1:0]            http_request_tdata          ,
  output wire [C_HTTP_REQUEST_TDATA_WIDTH/8-1:0]          http_request_tkeep          ,
  output wire                                             http_request_tlast          ,
  
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
  input  wire [C_HTTP_RESPONSE_TDATA_WIDTH/8-1:0]         http_response_tkeep         ,
  input  wire                                             http_response_tlast         ,
  
  input  wire                                             http_response_headers_tvalid,
  output wire                                             http_response_headers_tready,
  input  wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH-1:0]   http_response_headers_tdata ,
  input  wire [C_HTTP_RESPONSE_HEADERS_TDATA_WIDTH/8-1:0] http_response_headers_tkeep ,
  input  wire                                             http_response_headers_tlast ,
  
  input  wire                                             http_response_body_tvalid   ,
  output wire                                             http_response_body_tready   ,
  input  wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH-1:0]      http_response_body_tdata    ,
  input  wire [C_HTTP_RESPONSE_BODY_TDATA_WIDTH/8-1:0]    http_response_body_tkeep    ,
  input  wire                                             http_response_body_tlast    ,
  
  //
  // AXI4-LITE CONTROL
  //

  input  wire                                             s_axi_control_awvalid       ,
  output wire                                             s_axi_control_awready       ,
  input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]            s_axi_control_awaddr        ,
  input  wire                                             s_axi_control_wvalid        ,
  output wire                                             s_axi_control_wready        ,
  input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]            s_axi_control_wdata         ,
  input  wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0]          s_axi_control_wstrb         ,
  input  wire                                             s_axi_control_arvalid       ,
  output wire                                             s_axi_control_arready       ,
  input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]            s_axi_control_araddr        ,
  output wire                                             s_axi_control_rvalid        ,
  input  wire                                             s_axi_control_rready        ,
  output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]            s_axi_control_rdata         ,
  output wire [2-1:0]                                     s_axi_control_rresp         ,
  output wire                                             s_axi_control_bvalid        ,
  input  wire                                             s_axi_control_bready        ,
  output wire [2-1:0]                                     s_axi_control_bresp         ,
  output wire                                             interrupt
);

(* DONT_TOUCH = "yes" *)
reg  areset = 1'b0;
wire ap_start;
wire ap_idle;
wire ap_done;
wire ap_ready;
wire ap_continue;

always @(posedge ap_clk) begin
  areset <= ~ap_rst_n;
end

///////////////////////////////////////////////////////////////////////////////
// AXI4-Lite Control
///////////////////////////////////////////////////////////////////////////////

http_control_s_axi #(
  .C_S_AXI_ADDR_WIDTH ( C_S_AXI_CONTROL_ADDR_WIDTH ),
  .C_S_AXI_DATA_WIDTH ( C_S_AXI_CONTROL_DATA_WIDTH )
)
inst_control_s_axi (
  .ACLK        ( ap_clk                ),
  .ARESET      ( areset                ),
  .ACLK_EN     ( 1'b1                  ),
  .AWVALID     ( s_axi_control_awvalid ),
  .AWREADY     ( s_axi_control_awready ),
  .AWADDR      ( s_axi_control_awaddr  ),
  .WVALID      ( s_axi_control_wvalid  ),
  .WREADY      ( s_axi_control_wready  ),
  .WDATA       ( s_axi_control_wdata   ),
  .WSTRB       ( s_axi_control_wstrb   ),
  .ARVALID     ( s_axi_control_arvalid ),
  .ARREADY     ( s_axi_control_arready ),
  .ARADDR      ( s_axi_control_araddr  ),
  .RVALID      ( s_axi_control_rvalid  ),
  .RREADY      ( s_axi_control_rready  ),
  .RDATA       ( s_axi_control_rdata   ),
  .RRESP       ( s_axi_control_rresp   ),
  .BVALID      ( s_axi_control_bvalid  ),
  .BREADY      ( s_axi_control_bready  ),
  .BRESP       ( s_axi_control_bresp   ),
  .interrupt   ( interrupt             ),
  .ap_start    ( ap_start              ),
  .ap_done     ( ap_done               ),
  .ap_ready    ( ap_ready              ),
  .ap_idle     ( ap_idle               ),
  .ap_continue ( ap_continue           )
);

///////////////////////////////////////////////////////////////////////////////
// HTTP
///////////////////////////////////////////////////////////////////////////////

wire tcp_rx_data_V_data_V_TREADY;
wire tcp_rx_data_V_keep_V_TREADY;
wire tcp_rx_data_V_strb_V_TREADY;
wire tcp_rx_data_V_last_V_TREADY;
assign tcp_rx_data_tready = tcp_rx_data_V_data_V_TREADY & tcp_rx_data_V_keep_V_TREADY & tcp_rx_data_V_strb_V_TREADY & tcp_rx_data_V_last_V_TREADY;

wire tcp_tx_data_V_data_V_TVALID;
wire tcp_tx_data_V_keep_V_TVALID;
wire tcp_tx_data_V_strb_V_TVALID;
wire tcp_tx_data_V_last_V_TVALID;
assign tcp_tx_data_tvalid = tcp_tx_data_V_data_V_TVALID & tcp_tx_data_V_keep_V_TVALID & tcp_tx_data_V_strb_V_TVALID & tcp_tx_data_V_last_V_TVALID;

wire http_request_headers_V_data_V_TVALID;
wire http_request_headers_V_keep_V_TVALID;
wire http_request_headers_V_strb_V_TVALID;
wire http_request_headers_V_last_V_TVALID;
assign http_request_headers_tvalid = http_request_headers_V_data_V_TVALID & http_request_headers_V_keep_V_TVALID & http_request_headers_V_strb_V_TVALID & http_request_headers_V_last_V_TVALID;

assign http_request_tkeep = {C_HTTP_REQUEST_TDATA_WIDTH/8{1'b1}};
assign http_request_tlast = 1'b1;

wire http_request_body_V_data_V_TVALID;
wire http_request_body_V_keep_V_TVALID;
wire http_request_body_V_strb_V_TVALID;
wire http_request_body_V_last_V_TVALID;
assign http_request_body_tvalid = http_request_body_V_data_V_TVALID & http_request_body_V_keep_V_TVALID & http_request_body_V_strb_V_TVALID & http_request_body_V_last_V_TVALID;

wire http_response_headers_V_data_V_TREADY;
wire http_response_headers_V_keep_V_TREADY;
wire http_response_headers_V_strb_V_TREADY;
wire http_response_headers_V_last_V_TREADY;
assign http_response_headers_tready = http_response_headers_V_data_V_TREADY & http_response_headers_V_keep_V_TREADY & http_response_headers_V_strb_V_TREADY & http_response_headers_V_last_V_TREADY;

wire http_response_body_V_data_V_TREADY;
wire http_response_body_V_keep_V_TREADY;
wire http_response_body_V_strb_V_TREADY;
wire http_response_body_V_last_V_TREADY;
assign http_response_body_tready = http_response_body_V_data_V_TREADY & http_response_body_V_keep_V_TREADY & http_response_body_V_strb_V_TREADY & http_response_body_V_last_V_TREADY;

http_ip http_inst (
  .ap_clk(ap_clk),
  .ap_rst_n(ap_rst_n),

  //
  // TCP-IP
  //

  .tcp_listen_req_TDATA(tcp_listen_port_tdata),
  .tcp_listen_req_TVALID(tcp_listen_port_tvalid),
  .tcp_listen_req_TREADY(tcp_listen_port_tready),

  .tcp_listen_rsp_TDATA(tcp_port_status_tdata),
  .tcp_listen_rsp_TVALID(tcp_port_status_tvalid),
  .tcp_listen_rsp_TREADY(tcp_port_status_tready),

  .tcp_notification_TDATA(tcp_notification_tdata),
  .tcp_notification_TVALID(tcp_notification_tvalid),
  .tcp_notification_TREADY(tcp_notification_tready),

  .tcp_read_request_TDATA(tcp_read_pkg_tdata),
  .tcp_read_request_TVALID(tcp_read_pkg_tvalid),
  .tcp_read_request_TREADY(tcp_read_pkg_tready),

  .tcp_rx_meta_TDATA(tcp_rx_meta_tdata),
  .tcp_rx_meta_TVALID(tcp_rx_meta_tvalid),
  .tcp_rx_meta_TREADY(tcp_rx_meta_tready),

  .tcp_tx_meta_TDATA(tcp_tx_meta_tdata),
  .tcp_tx_meta_TVALID(tcp_tx_meta_tvalid),
  .tcp_tx_meta_TREADY(tcp_tx_meta_tready),

  .tcp_tx_status_TDATA(tcp_tx_status_tdata),
  .tcp_tx_status_TVALID(tcp_tx_status_tvalid),
  .tcp_tx_status_TREADY(tcp_tx_status_tready),

  .tcp_rx_data_V_data_V_TDATA(tcp_rx_data_tdata),
  .tcp_rx_data_V_keep_V_TKEEP(tcp_rx_data_tkeep),
  .tcp_rx_data_V_last_V_TLAST(tcp_rx_data_tlast),
  .tcp_rx_data_V_strb_V_TSTRB(tcp_rx_data_tkeep),
  .tcp_rx_data_V_data_V_TVALID(tcp_rx_data_tvalid),
  .tcp_rx_data_V_data_V_TREADY(tcp_rx_data_V_data_V_TREADY),
  .tcp_rx_data_V_keep_V_TVALID(tcp_rx_data_tvalid),
  .tcp_rx_data_V_keep_V_TREADY(tcp_rx_data_V_keep_V_TREADY),
  .tcp_rx_data_V_strb_V_TVALID(tcp_rx_data_tvalid),
  .tcp_rx_data_V_strb_V_TREADY(tcp_rx_data_V_strb_V_TREADY),
  .tcp_rx_data_V_last_V_TVALID(tcp_rx_data_tvalid),
  .tcp_rx_data_V_last_V_TREADY(tcp_rx_data_V_last_V_TREADY),

  .tcp_tx_data_V_data_V_TDATA(tcp_tx_data_tdata),
  .tcp_tx_data_V_keep_V_TKEEP(tcp_tx_data_tkeep),
  .tcp_tx_data_V_last_V_TLAST(tcp_tx_data_tlast),
  .tcp_tx_data_V_strb_V_TSTRB(tcp_tx_data_tkeep),
  .tcp_tx_data_V_data_V_TVALID(tcp_tx_data_V_data_V_TVALID),
  .tcp_tx_data_V_data_V_TREADY(tcp_tx_data_tready),
  .tcp_tx_data_V_keep_V_TVALID(tcp_tx_data_V_keep_V_TVALID),
  .tcp_tx_data_V_keep_V_TREADY(tcp_tx_data_tready),
  .tcp_tx_data_V_strb_V_TVALID(tcp_tx_data_V_strb_V_TVALID),
  .tcp_tx_data_V_strb_V_TREADY(tcp_tx_data_tready),
  .tcp_tx_data_V_last_V_TVALID(tcp_tx_data_V_last_V_TVALID),
  .tcp_tx_data_V_last_V_TREADY(tcp_tx_data_tready),

  //
  // APPLICATION
  //
  
  .http_request_TDATA(http_request_tdata),
  .http_request_TVALID(http_request_tvalid),
  .http_request_TREADY(http_request_tready),

  .http_request_headers_V_data_V_TDATA(http_request_headers_tdata),
  .http_request_headers_V_keep_V_TKEEP(http_request_headers_tkeep),
  .http_request_headers_V_last_V_TLAST(http_request_headers_tlast),
  .http_request_headers_V_strb_V_TSTRB(http_request_headers_tkeep),
  .http_request_headers_V_data_V_TVALID(http_request_headers_V_data_V_TVALID),
  .http_request_headers_V_data_V_TREADY(http_request_headers_tready),
  .http_request_headers_V_keep_V_TVALID(http_request_headers_V_keep_V_TVALID),
  .http_request_headers_V_keep_V_TREADY(http_request_headers_tready),
  .http_request_headers_V_strb_V_TVALID(http_request_headers_V_strb_V_TVALID),
  .http_request_headers_V_strb_V_TREADY(http_request_headers_tready),
  .http_request_headers_V_last_V_TVALID(http_request_headers_V_last_V_TVALID),
  .http_request_headers_V_last_V_TREADY(http_request_headers_tready),

  .http_request_body_V_data_V_TDATA(http_request_body_tdata),
  .http_request_body_V_keep_V_TKEEP(http_request_body_tkeep),
  .http_request_body_V_last_V_TLAST(http_request_body_tlast),
  .http_request_body_V_strb_V_TSTRB(http_request_body_tkeep),
  .http_request_body_V_data_V_TVALID(http_request_body_V_data_V_TVALID),
  .http_request_body_V_data_V_TREADY(http_request_body_tready),
  .http_request_body_V_keep_V_TVALID(http_request_body_V_keep_V_TVALID),
  .http_request_body_V_keep_V_TREADY(http_request_body_tready),
  .http_request_body_V_strb_V_TVALID(http_request_body_V_strb_V_TVALID),
  .http_request_body_V_strb_V_TREADY(http_request_body_tready),
  .http_request_body_V_last_V_TVALID(http_request_body_V_last_V_TVALID),
  .http_request_body_V_last_V_TREADY(http_request_body_tready),

  .http_response_TDATA(http_response_tdata),
  .http_response_TVALID(http_response_tvalid),
  .http_response_TREADY(http_response_tready),

  .http_response_headers_V_data_V_TDATA(http_response_headers_tdata),
  .http_response_headers_V_keep_V_TKEEP(http_response_headers_tkeep),
  .http_response_headers_V_last_V_TLAST(http_response_headers_tlast),
  .http_response_headers_V_strb_V_TSTRB(http_response_headers_tkeep),
  .http_response_headers_V_data_V_TVALID(http_response_headers_tvalid),
  .http_response_headers_V_data_V_TREADY(http_response_headers_V_data_V_TREADY),
  .http_response_headers_V_keep_V_TVALID(http_response_headers_tvalid),
  .http_response_headers_V_keep_V_TREADY(http_response_headers_V_keep_V_TREADY),
  .http_response_headers_V_strb_V_TVALID(http_response_headers_tvalid),
  .http_response_headers_V_strb_V_TREADY(http_response_headers_V_strb_V_TREADY),
  .http_response_headers_V_last_V_TVALID(http_response_headers_tvalid),
  .http_response_headers_V_last_V_TREADY(http_response_headers_V_last_V_TREADY),

  .http_response_body_V_data_V_TDATA(http_response_body_tdata),
  .http_response_body_V_keep_V_TKEEP(http_response_body_tkeep),
  .http_response_body_V_last_V_TLAST(http_response_body_tlast),
  .http_response_body_V_strb_V_TSTRB(http_response_body_tkeep),
  .http_response_body_V_data_V_TVALID(http_response_body_tvalid),
  .http_response_body_V_data_V_TREADY(http_response_body_V_data_V_TREADY),
  .http_response_body_V_keep_V_TVALID(http_response_body_tvalid),
  .http_response_body_V_keep_V_TREADY(http_response_body_V_keep_V_TREADY),
  .http_response_body_V_strb_V_TVALID(http_response_body_tvalid),
  .http_response_body_V_strb_V_TREADY(http_response_body_V_strb_V_TREADY),
  .http_response_body_V_last_V_TVALID(http_response_body_tvalid),
  .http_response_body_V_last_V_TREADY(http_response_body_V_last_V_TREADY)
);

///////////////////////////////////////////////////////////////////////////////
// UNUSED SIGNALS
///////////////////////////////////////////////////////////////////////////////

assign tcp_listen_port_tkeep = {C_TCP_LISTEN_PORT_TDATA_WIDTH/8{1'b1}};
assign tcp_listen_port_tlast = 1'b1;

assign tcp_open_conn_tvalid = 1'b0;
assign tcp_open_conn_tdata = {C_TCP_OPEN_CONN_TDATA_WIDTH{1'b0}};
assign tcp_open_conn_tkeep = {C_TCP_OPEN_CONN_TDATA_WIDTH/8{1'b0}};
assign tcp_open_conn_tlast = 1'b1;

assign tcp_open_status_tready = 1'b0;

assign tcp_close_conn_tvalid = 1'b0;
assign tcp_close_conn_tdata = {C_TCP_CLOSE_CONN_TDATA_WIDTH{1'b0}};
assign tcp_close_conn_tkeep = {C_TCP_CLOSE_CONN_TDATA_WIDTH/8{1'b0}};
assign tcp_close_conn_tlast = 1'b1;

assign tcp_read_pkg_tkeep = {C_TCP_READ_PKG_TDATA_WIDTH/8{1'b1}};
assign tcp_read_pkg_tlast = 1'b1;

assign tcp_tx_meta_tkeep = {C_TCP_TX_META_TDATA_WIDTH/8{1'b1}};
assign tcp_tx_meta_tlast = 1'b1;

assign udp_rx_tready = 1'b0;
assign udp_tx_tvalid = 1'b0;
assign udp_tx_tdata = {C_UDP_TX_TDATA_WIDTH{1'b0}};
assign udp_tx_tkeep = {C_UDP_TX_TDATA_WIDTH/8{1'b0}};
assign udp_tx_tlast = 1'b1;
assign udp_rx_meta_tready = 1'b0;
assign udp_tx_meta_tvalid = 1'b0;
assign udp_tx_meta_tdata = {C_UDP_TX_META_TDATA_WIDTH{1'b0}};
assign udp_tx_meta_tkeep = {C_UDP_TX_META_TDATA_WIDTH/8{1'b0}};
assign udp_tx_meta_tlast = 1'b0;

endmodule
`default_nettype wire