
`timescale 1ns / 100ps

module VexiiRiscv_wrapper (
    input  wire EmbeddedRiscvJtag_logic_jtag_tms,
    input  wire EmbeddedRiscvJtag_logic_jtag_tdi,
    output wire EmbeddedRiscvJtag_logic_jtag_tdo,
    input  wire EmbeddedRiscvJtag_logic_jtag_tck,
    output wire EmbeddedRiscvJtag_logic_ndmreset,

    input wire [63:0] PrivilegedPlugin_logic_rdtime,
    input wire PrivilegedPlugin_logic_harts_0_int_m_timer,
    input wire PrivilegedPlugin_logic_harts_0_int_m_software,
    input wire        PrivilegedPlugin_logic_harts_0_int_m_external,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWVALID" *)
    output wire        LsuL1Axi4Plugin_logic_axi_aw_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWREADY" *)
    input  wire        LsuL1Axi4Plugin_logic_axi_aw_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWADDR" *)
    output wire [31:0] LsuL1Axi4Plugin_logic_axi_aw_payload_addr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWLEN" *)
    output wire [ 7:0] LsuL1Axi4Plugin_logic_axi_aw_payload_len,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWSIZE" *)
    output wire [ 2:0] LsuL1Axi4Plugin_logic_axi_aw_payload_size,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWBURST" *)
    output wire [ 1:0] LsuL1Axi4Plugin_logic_axi_aw_payload_burst,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWCACHE" *)
    output wire [ 3:0] LsuL1Axi4Plugin_logic_axi_aw_payload_cache,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 AWPROT" *)
    output wire [ 2:0] LsuL1Axi4Plugin_logic_axi_aw_payload_prot,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 WVALID" *)
    output wire        LsuL1Axi4Plugin_logic_axi_w_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 WREADY" *)
    input  wire        LsuL1Axi4Plugin_logic_axi_w_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 WDATA" *)
    output wire [31:0] LsuL1Axi4Plugin_logic_axi_w_payload_data,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 WSTRB" *)
    output wire [ 3:0] LsuL1Axi4Plugin_logic_axi_w_payload_strb,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 WLAST" *)
    output wire        LsuL1Axi4Plugin_logic_axi_w_payload_last,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 BVALID" *)
    input  wire        LsuL1Axi4Plugin_logic_axi_b_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 BREADY" *)
    output wire        LsuL1Axi4Plugin_logic_axi_b_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 BRESP" *)
    input  wire [ 1:0] LsuL1Axi4Plugin_logic_axi_b_payload_resp,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARVALID" *)
    output wire        LsuL1Axi4Plugin_logic_axi_ar_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARREADY" *)
    input  wire        LsuL1Axi4Plugin_logic_axi_ar_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARADDR" *)
    output wire [31:0] LsuL1Axi4Plugin_logic_axi_ar_payload_addr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARLEN" *)
    output wire [ 7:0] LsuL1Axi4Plugin_logic_axi_ar_payload_len,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARSIZE" *)
    output wire [ 2:0] LsuL1Axi4Plugin_logic_axi_ar_payload_size,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARBURST" *)
    output wire [ 1:0] LsuL1Axi4Plugin_logic_axi_ar_payload_burst,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARCACHE" *)
    output wire [ 3:0] LsuL1Axi4Plugin_logic_axi_ar_payload_cache,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 ARPROT" *)
    output wire [ 2:0] LsuL1Axi4Plugin_logic_axi_ar_payload_prot,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 RVALID" *)
    input  wire        LsuL1Axi4Plugin_logic_axi_r_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 RREADY" *)
    output wire        LsuL1Axi4Plugin_logic_axi_r_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 RDATA" *)
    input  wire [31:0] LsuL1Axi4Plugin_logic_axi_r_payload_data,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 RRESP" *)
    input  wire [ 1:0] LsuL1Axi4Plugin_logic_axi_r_payload_resp,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuL1 RLAST" *)
    input  wire        LsuL1Axi4Plugin_logic_axi_r_payload_last,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARVALID" *)
    output wire        FetchL1Axi4Plugin_logic_axi_ar_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARREADY" *)
    input  wire        FetchL1Axi4Plugin_logic_axi_ar_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARADDR" *)
    output wire [31:0] FetchL1Axi4Plugin_logic_axi_ar_payload_addr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARLEN" *)
    output wire [ 7:0] FetchL1Axi4Plugin_logic_axi_ar_payload_len,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARSIZE" *)
    output wire [ 2:0] FetchL1Axi4Plugin_logic_axi_ar_payload_size,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARBURST" *)
    output wire [ 1:0] FetchL1Axi4Plugin_logic_axi_ar_payload_burst,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARCACHE" *)
    output wire [ 3:0] FetchL1Axi4Plugin_logic_axi_ar_payload_cache,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 ARPROT" *)
    output wire [ 2:0] FetchL1Axi4Plugin_logic_axi_ar_payload_prot,
    
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 RVALID" *)
    input  wire        FetchL1Axi4Plugin_logic_axi_r_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 RREADY" *)
    output wire        FetchL1Axi4Plugin_logic_axi_r_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 RDATA" *)
    input  wire [31:0] FetchL1Axi4Plugin_logic_axi_r_payload_data,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 RRESP" *)
    input  wire [ 1:0] FetchL1Axi4Plugin_logic_axi_r_payload_resp,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 FetchL1 RLAST" *)
    input  wire        FetchL1Axi4Plugin_logic_axi_r_payload_last,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless AWVALID" *)
    output wire        LsuCachelessAxi4Plugin_logic_axi_aw_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless AWREADY" *)
    input  wire        LsuCachelessAxi4Plugin_logic_axi_aw_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless AWADDR" *)
    output wire [31:0] LsuCachelessAxi4Plugin_logic_axi_aw_payload_addr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless AWSIZE" *)
    output wire [ 2:0] LsuCachelessAxi4Plugin_logic_axi_aw_payload_size,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless AWCACHE" *)
    output wire [ 3:0] LsuCachelessAxi4Plugin_logic_axi_aw_payload_cache,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless AWPROT" *)
    output wire [ 2:0] LsuCachelessAxi4Plugin_logic_axi_aw_payload_prot,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless WVALID" *)
    output wire        LsuCachelessAxi4Plugin_logic_axi_w_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless WREADY" *)
    input  wire        LsuCachelessAxi4Plugin_logic_axi_w_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless WDATA" *)
    output wire [31:0] LsuCachelessAxi4Plugin_logic_axi_w_payload_data,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless WSTRB" *)
    output wire [ 3:0] LsuCachelessAxi4Plugin_logic_axi_w_payload_strb,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless WLAST" *)
    output wire        LsuCachelessAxi4Plugin_logic_axi_w_payload_last,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless BVALID" *)
    input  wire        LsuCachelessAxi4Plugin_logic_axi_b_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless BREADY" *)
    output wire        LsuCachelessAxi4Plugin_logic_axi_b_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless BRESP" *)
    input  wire [ 1:0] LsuCachelessAxi4Plugin_logic_axi_b_payload_resp,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless ARVALID" *)
    output wire        LsuCachelessAxi4Plugin_logic_axi_ar_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless ARREADY" *)
    input  wire        LsuCachelessAxi4Plugin_logic_axi_ar_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless ARADDR" *)
    output wire [31:0] LsuCachelessAxi4Plugin_logic_axi_ar_payload_addr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless ARSIZE" *)
    output wire [ 2:0] LsuCachelessAxi4Plugin_logic_axi_ar_payload_size,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless ARCACHE" *)
    output wire [ 3:0] LsuCachelessAxi4Plugin_logic_axi_ar_payload_cache,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless ARPROT" *)
    output wire [ 2:0] LsuCachelessAxi4Plugin_logic_axi_ar_payload_prot,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless RVALID" *)
    input  wire        LsuCachelessAxi4Plugin_logic_axi_r_valid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless RREADY" *)
    output wire        LsuCachelessAxi4Plugin_logic_axi_r_ready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless RDATA" *)
    input  wire [31:0] LsuCachelessAxi4Plugin_logic_axi_r_payload_data,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless RRESP" *)
    input  wire [ 1:0] LsuCachelessAxi4Plugin_logic_axi_r_payload_resp,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 LsuCacheless RLAST" *)
    input  wire        LsuCachelessAxi4Plugin_logic_axi_r_payload_last,

    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 clk CLK" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_RESET reset, ASSOCIATED_BUSIF LsuCacheless:FetchL1:LsuL1" *)
    input  wire        clk,
    
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 reset RST" *)
    (* X_INTERFACE_PARAMETER = "POLARITY ACTIVE_HIGH" *)
    input wire reset
);

    // Fixes to access Zynq DRR via HPx ports
    // Or you get DECERR or SLVERR responses
    assign LsuL1Axi4Plugin_logic_axi_aw_payload_cache   = 4'b0011;
    assign LsuL1Axi4Plugin_logic_axi_ar_payload_cache   = 4'b0011;
    assign FetchL1Axi4Plugin_logic_axi_ar_payload_cache = 4'b0011;

    assign LsuL1Axi4Plugin_logic_axi_aw_payload_prot   = 3'b000;
    assign LsuL1Axi4Plugin_logic_axi_ar_payload_prot   = 3'b000;
    assign FetchL1Axi4Plugin_logic_axi_ar_payload_prot = 3'b000;

    VexiiRiscv cpu (
        .EmbeddedRiscvJtag_logic_jtag_tms(EmbeddedRiscvJtag_logic_jtag_tms),
        .EmbeddedRiscvJtag_logic_jtag_tdi(EmbeddedRiscvJtag_logic_jtag_tdi),
        .EmbeddedRiscvJtag_logic_jtag_tdo(EmbeddedRiscvJtag_logic_jtag_tdo),
        .EmbeddedRiscvJtag_logic_jtag_tck(EmbeddedRiscvJtag_logic_jtag_tck),
        .EmbeddedRiscvJtag_logic_ndmreset(EmbeddedRiscvJtag_logic_ndmreset),
        .PrivilegedPlugin_logic_rdtime(PrivilegedPlugin_logic_rdtime),
        .PrivilegedPlugin_logic_harts_0_int_m_timer(PrivilegedPlugin_logic_harts_0_int_m_timer),
        .PrivilegedPlugin_logic_harts_0_int_m_software(PrivilegedPlugin_logic_harts_0_int_m_software),
        .PrivilegedPlugin_logic_harts_0_int_m_external(PrivilegedPlugin_logic_harts_0_int_m_external),
        .LsuL1Axi4Plugin_logic_axi_aw_valid(LsuL1Axi4Plugin_logic_axi_aw_valid),
        .LsuL1Axi4Plugin_logic_axi_aw_ready(LsuL1Axi4Plugin_logic_axi_aw_ready),
        .LsuL1Axi4Plugin_logic_axi_aw_payload_addr(LsuL1Axi4Plugin_logic_axi_aw_payload_addr),
        .LsuL1Axi4Plugin_logic_axi_aw_payload_len(LsuL1Axi4Plugin_logic_axi_aw_payload_len),
        .LsuL1Axi4Plugin_logic_axi_aw_payload_size(LsuL1Axi4Plugin_logic_axi_aw_payload_size),
        .LsuL1Axi4Plugin_logic_axi_aw_payload_burst(LsuL1Axi4Plugin_logic_axi_aw_payload_burst),
        .LsuL1Axi4Plugin_logic_axi_aw_payload_cache(),
        .LsuL1Axi4Plugin_logic_axi_aw_payload_prot(),
        .LsuL1Axi4Plugin_logic_axi_w_valid(LsuL1Axi4Plugin_logic_axi_w_valid),
        .LsuL1Axi4Plugin_logic_axi_w_ready(LsuL1Axi4Plugin_logic_axi_w_ready),
        .LsuL1Axi4Plugin_logic_axi_w_payload_data(LsuL1Axi4Plugin_logic_axi_w_payload_data),
        .LsuL1Axi4Plugin_logic_axi_w_payload_strb(LsuL1Axi4Plugin_logic_axi_w_payload_strb),
        .LsuL1Axi4Plugin_logic_axi_w_payload_last(LsuL1Axi4Plugin_logic_axi_w_payload_last),
        .LsuL1Axi4Plugin_logic_axi_b_valid(LsuL1Axi4Plugin_logic_axi_b_valid),
        .LsuL1Axi4Plugin_logic_axi_b_ready(LsuL1Axi4Plugin_logic_axi_b_ready),
        .LsuL1Axi4Plugin_logic_axi_b_payload_resp(LsuL1Axi4Plugin_logic_axi_b_payload_resp),
        .LsuL1Axi4Plugin_logic_axi_ar_valid(LsuL1Axi4Plugin_logic_axi_ar_valid),
        .LsuL1Axi4Plugin_logic_axi_ar_ready(LsuL1Axi4Plugin_logic_axi_ar_ready),
        .LsuL1Axi4Plugin_logic_axi_ar_payload_addr(LsuL1Axi4Plugin_logic_axi_ar_payload_addr),
        .LsuL1Axi4Plugin_logic_axi_ar_payload_len(LsuL1Axi4Plugin_logic_axi_ar_payload_len),
        .LsuL1Axi4Plugin_logic_axi_ar_payload_size(LsuL1Axi4Plugin_logic_axi_ar_payload_size),
        .LsuL1Axi4Plugin_logic_axi_ar_payload_burst(LsuL1Axi4Plugin_logic_axi_ar_payload_burst),
        .LsuL1Axi4Plugin_logic_axi_ar_payload_cache(),
        .LsuL1Axi4Plugin_logic_axi_ar_payload_prot(),
        .LsuL1Axi4Plugin_logic_axi_r_valid(LsuL1Axi4Plugin_logic_axi_r_valid),
        .LsuL1Axi4Plugin_logic_axi_r_ready(LsuL1Axi4Plugin_logic_axi_r_ready),
        .LsuL1Axi4Plugin_logic_axi_r_payload_data(LsuL1Axi4Plugin_logic_axi_r_payload_data),
        .LsuL1Axi4Plugin_logic_axi_r_payload_resp(LsuL1Axi4Plugin_logic_axi_r_payload_resp),
        .LsuL1Axi4Plugin_logic_axi_r_payload_last(LsuL1Axi4Plugin_logic_axi_r_payload_last),
        .FetchL1Axi4Plugin_logic_axi_ar_valid(FetchL1Axi4Plugin_logic_axi_ar_valid),
        .FetchL1Axi4Plugin_logic_axi_ar_ready(FetchL1Axi4Plugin_logic_axi_ar_ready),
        .FetchL1Axi4Plugin_logic_axi_ar_payload_addr(FetchL1Axi4Plugin_logic_axi_ar_payload_addr),
        .FetchL1Axi4Plugin_logic_axi_ar_payload_len(FetchL1Axi4Plugin_logic_axi_ar_payload_len),
        .FetchL1Axi4Plugin_logic_axi_ar_payload_size(FetchL1Axi4Plugin_logic_axi_ar_payload_size),
        .FetchL1Axi4Plugin_logic_axi_ar_payload_burst(FetchL1Axi4Plugin_logic_axi_ar_payload_burst),
        .FetchL1Axi4Plugin_logic_axi_ar_payload_cache(),
        .FetchL1Axi4Plugin_logic_axi_ar_payload_prot(),
        .FetchL1Axi4Plugin_logic_axi_r_valid(FetchL1Axi4Plugin_logic_axi_r_valid),
        .FetchL1Axi4Plugin_logic_axi_r_ready(FetchL1Axi4Plugin_logic_axi_r_ready),
        .FetchL1Axi4Plugin_logic_axi_r_payload_data(FetchL1Axi4Plugin_logic_axi_r_payload_data),
        .FetchL1Axi4Plugin_logic_axi_r_payload_resp(FetchL1Axi4Plugin_logic_axi_r_payload_resp),
        .FetchL1Axi4Plugin_logic_axi_r_payload_last(FetchL1Axi4Plugin_logic_axi_r_payload_last),
        .LsuCachelessAxi4Plugin_logic_axi_aw_valid(LsuCachelessAxi4Plugin_logic_axi_aw_valid),
        .LsuCachelessAxi4Plugin_logic_axi_aw_ready(LsuCachelessAxi4Plugin_logic_axi_aw_ready),
        .LsuCachelessAxi4Plugin_logic_axi_aw_payload_addr(LsuCachelessAxi4Plugin_logic_axi_aw_payload_addr),
        .LsuCachelessAxi4Plugin_logic_axi_aw_payload_size(LsuCachelessAxi4Plugin_logic_axi_aw_payload_size),
        .LsuCachelessAxi4Plugin_logic_axi_aw_payload_cache(LsuCachelessAxi4Plugin_logic_axi_aw_payload_cache),
        .LsuCachelessAxi4Plugin_logic_axi_aw_payload_prot(LsuCachelessAxi4Plugin_logic_axi_aw_payload_prot),
        .LsuCachelessAxi4Plugin_logic_axi_w_valid(LsuCachelessAxi4Plugin_logic_axi_w_valid),
        .LsuCachelessAxi4Plugin_logic_axi_w_ready(LsuCachelessAxi4Plugin_logic_axi_w_ready),
        .LsuCachelessAxi4Plugin_logic_axi_w_payload_data(LsuCachelessAxi4Plugin_logic_axi_w_payload_data),
        .LsuCachelessAxi4Plugin_logic_axi_w_payload_strb(LsuCachelessAxi4Plugin_logic_axi_w_payload_strb),
        .LsuCachelessAxi4Plugin_logic_axi_w_payload_last(LsuCachelessAxi4Plugin_logic_axi_w_payload_last),
        .LsuCachelessAxi4Plugin_logic_axi_b_valid(LsuCachelessAxi4Plugin_logic_axi_b_valid),
        .LsuCachelessAxi4Plugin_logic_axi_b_ready(LsuCachelessAxi4Plugin_logic_axi_b_ready),
        .LsuCachelessAxi4Plugin_logic_axi_b_payload_resp(LsuCachelessAxi4Plugin_logic_axi_b_payload_resp),
        .LsuCachelessAxi4Plugin_logic_axi_ar_valid(LsuCachelessAxi4Plugin_logic_axi_ar_valid),
        .LsuCachelessAxi4Plugin_logic_axi_ar_ready(LsuCachelessAxi4Plugin_logic_axi_ar_ready),
        .LsuCachelessAxi4Plugin_logic_axi_ar_payload_addr(LsuCachelessAxi4Plugin_logic_axi_ar_payload_addr),
        .LsuCachelessAxi4Plugin_logic_axi_ar_payload_size(LsuCachelessAxi4Plugin_logic_axi_ar_payload_size),
        .LsuCachelessAxi4Plugin_logic_axi_ar_payload_cache(LsuCachelessAxi4Plugin_logic_axi_ar_payload_cache),
        .LsuCachelessAxi4Plugin_logic_axi_ar_payload_prot(LsuCachelessAxi4Plugin_logic_axi_ar_payload_prot),
        .LsuCachelessAxi4Plugin_logic_axi_r_valid(LsuCachelessAxi4Plugin_logic_axi_r_valid),
        .LsuCachelessAxi4Plugin_logic_axi_r_ready(LsuCachelessAxi4Plugin_logic_axi_r_ready),
        .LsuCachelessAxi4Plugin_logic_axi_r_payload_data(LsuCachelessAxi4Plugin_logic_axi_r_payload_data),
        .LsuCachelessAxi4Plugin_logic_axi_r_payload_resp(LsuCachelessAxi4Plugin_logic_axi_r_payload_resp),
        .LsuCachelessAxi4Plugin_logic_axi_r_payload_last(LsuCachelessAxi4Plugin_logic_axi_r_payload_last),
        .clk(clk),
        .reset(reset)
    );

endmodule
