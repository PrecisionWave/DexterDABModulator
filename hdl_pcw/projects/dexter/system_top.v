// ***************************************************************************
// ***************************************************************************
// Copyright 2022(c) PrecisionWave AG
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//     - Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     - Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in
//       the documentation and/or other materials provided with the
//       distribution.
//     - Neither the name of PrecisionWave AG nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//     - The use of this software may or may not infringe the patent rights
//       of one or more patent holders.  This license does not release you
//       from the requirement that you obtain separate licenses from these
//       patent holders to use this software.
//     - Use of the software either in source or binary form, must be run
//       on or directly connected to an Analog Devices Inc. component.
//
// THIS SOFTWARE IS PROVIDED BY PRECISIONWAVE "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED.
//
// IN NO EVENT SHALL PRECISIONWAVE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
// RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

`timescale 1ns/100ps

module system_top (
  inout   [14:0]  DDR_ADDR,
  inout   [ 2:0]  DDR_BA,
  inout           DDR_CAS_N,
  inout           DDR_CK_N,
  inout           DDR_CK_P,
  inout           DDR_CKE,
  inout           DDR_CS_N,
  inout   [ 3:0]  DDR_DM,
  inout   [31:0]  DDR_DQ,
  inout   [ 3:0]  DDR_DQS_N,
  inout   [ 3:0]  DDR_DQS_P,
  inout           DDR_ODT,
  inout           DDR_RAS_N,
  inout           DDR_RESET_N,
  inout           DDR_WE_N,

  inout           FIXED_IO_DDR_VRN,
  inout           FIXED_IO_DDR_VRP,
  inout   [53:0]  FIXED_IO_MIO,
  inout           FIXED_IO_PS_CLK,
  inout           FIXED_IO_PS_PORB,
  inout           FIXED_IO_PS_SRSTB,
  
  // clkgen LMK04805
  input           FPGA_CLK_P,
  input           FPGA_CLK_N,

  // DAC AD9957 digital upconverter
  input           DAC_PDCLK,
  output  [17:0]  DAC_D,
  output          DAC_TxEN,
  output          DAC_IO_RESET,
  output          DAC_IO_UPDATE,
  output          DAC_MR,

  // ADC AD9634
  input           ADC_DCO_P,
  input           ADC_DCO_N,
  input           ADC_OR_P,
  input           ADC_OR_N,
  input   [ 5:0]  ADC_D_P,
  input   [ 5:0]  ADC_D_N,

  // I2C
  input           I2C_INT_N,
  inout           I2C_SCL,
  inout           I2C_SDA,
  
  // PA 2x5Pin Connector
  inout           SCL_GPIO0,
  inout           SDA_GPIO1,

  // SPI
  input           SPI_MISO,
  output          SPI_MOSI,
  output          SPI_SCK,

  // SPI CS
  output          nCS_AD9833,   // DDS
  output          nCS_AD9957,   // DAC
  output          nCS_LMK,
  
  // ADC "SPI"
  output          ADC_SPI_CS,
  output          ADC_SPI_SCLK,
  inout           ADC_SPI_SDIO,
  
  // PLL
  input           Status_LD,
  input           Status_CLKin0,
  input           Status_CLKin1,

  // DDS
  output          extRef_nOCXO,

  // GPS
  inout           PPS_IO,
  input           GPS_PPS,
  input           GPS_TX,
  output          GPS_RX,

  // USB
  input           USB_OCn,
  
  // LEDs ZX2 module
  output   [3:0]  MODULE_LED,
  
  // Front
  input           Button_Front,
  output          ENn_LED_red,
  output          ENn_LED_green
);

  // internal signals

  wire            ext_pps_in;
  wire            ext_pps_out;
  wire            ext_pps_t;
  
  wire    [63:0]  gpio_i;
  wire    [63:0]  gpio_o;
  wire    [63:0]  gpio_t;
  
  wire    [ 2:0]  spi0_csn;
  wire            spi0_clk;
  wire            spi0_mosi;
  wire            spi0_miso;
  
  wire            i2c1_scl_o;
  wire            i2c1_scl_i;
  wire            i2c1_scl_t;
  wire            i2c1_sda_o;
  wire            i2c1_sda_i;
  wire            i2c1_sda_t;
  
  wire            sys_cpu_clk;
  wire            sys_cpu_resetn;
  
  wire    [17:0]  duc_i;
  reg     [17:0]  duc_q_reg;
  wire    [17:0]  duc_q;
  wire            duc_tx_en;
  wire            duc_pdclk;
  reg      [2:0]  duc_ss_reg;
  
  wire            fpga_clk_ubuf;
  wire            fpga_clk;
  
  wire    [11:0]  adc_d;
  wire    [11:0]  adc_d1;
  wire    [ 5:0]  adc_ddr;
  wire            adc_clk;
  wire            adc_ovf;
       
  parameter COUNTER_WIDTH = 26;
  reg [COUNTER_WIDTH-1:0] cnt_pdclk = {COUNTER_WIDTH{1'b0}};
  reg [COUNTER_WIDTH-1:0] cnt_adcclk = {COUNTER_WIDTH{1'b0}};
  
  genvar i;

      
   // LED counters ////////////////////////////////////////////////////
   always @(posedge duc_pdclk)
     cnt_pdclk <= cnt_pdclk + 1'b1;
   always @(posedge adc_clk)
     cnt_adcclk <= cnt_adcclk + 1'b1;

   // Front LEDs & Button /////////////////////////////////////////////
   //assign ENn_LED_green = cnt_pdclk[COUNTER_WIDTH-1];
   //assign ENn_LED_red = ext_pps_out;
   assign ENn_LED_red = gpio_o[8];
   assign ENn_LED_green = gpio_o[9];
   assign gpio_i[13] = Button_Front;
   
   // ZX2 MODULE LEDs /////////////////////////////////////////////////
   assign MODULE_LED[0] = cnt_pdclk[COUNTER_WIDTH-1];
   assign MODULE_LED[1] = cnt_adcclk[COUNTER_WIDTH-1];
   assign MODULE_LED[2] = GPS_PPS;
   assign MODULE_LED[3] = gpio_o[11];
  
   // OCXO disable, ext ref enable ////////////////////////////////////
   assign extRef_nOCXO = gpio_o[10];
  
   // PLL status //////////////////////////////////////////////////////
   assign gpio_i[20] = Status_LD;
   assign gpio_i[21] = Status_CLKin0;
   assign gpio_i[22] = Status_CLKin1;
   
   // SPI /////////////////////////////////////////////////////////////
   assign spi0_miso = SPI_MISO;          // LMK: Holdover
   assign SPI_MOSI = spi0_mosi;          // LMK: DATA
   assign SPI_SCK = spi0_clk;            // LMK: CLK
   assign nCS_LMK    = spi0_csn[0];      // LMK: LE
   assign nCS_AD9833 = spi0_csn[1];
  
   // SPI AD9957
   always @(posedge sys_cpu_clk) 
     duc_ss_reg <= {duc_ss_reg[1:0], spi0_csn[2]};
    
   assign nCS_AD9957 = duc_ss_reg[2];
   assign DAC_IO_RESET = duc_ss_reg[2] & ~duc_ss_reg[1]; // io reset pulse on falling slave select edge
   assign DAC_IO_UPDATE = ~duc_ss_reg[2] & duc_ss_reg[1]; // io reset pulse on rising slave select edge
   assign DAC_MR = ~sys_cpu_resetn;
   assign DAC_TxEN = duc_tx_en;
   
   // I2C Tristate drivers GPIO PA 2x5 Connector //////////////////////
   assign I2C_SCL = i2c1_scl_t ? 1'bz : i2c1_scl_o;
   assign I2C_SDA = i2c1_sda_t ? 1'bz : i2c1_sda_o;
   assign SCL_GPIO0 = i2c1_scl_t ? 1'bz : i2c1_scl_o;
   assign SDA_GPIO1 = i2c1_sda_t ? 1'bz : i2c1_sda_o;
   assign i2c1_scl_i = I2C_SCL & SCL_GPIO0;
   assign i2c1_sda_i = I2C_SDA & SDA_GPIO1;
  
   // GPS PPS gpio interface //////////////////////////////////////////
   assign gpio_i[25] = ext_pps_out;
   assign PPS_IO = ext_pps_t ? 1'bz : ext_pps_out;
   assign ext_pps_in = PPS_IO;
  
   // AD9957 digital upconverter/DAC IQ data /////////////////////////
   always @(posedge duc_pdclk) begin
      duc_q_reg <= duc_q;
   end

   generate
   for (i=0; i < 18; i=i+1) begin
     ODDR #(
       .DDR_CLK_EDGE("OPPOSITE_EDGE"),
       .INIT(1'b0),
       .SRTYPE("SYNC")
     ) ODDR_inst (
       .Q(DAC_D[i]),
       .C(duc_pdclk),
       .CE(1'b1),
       .D1(duc_i[i]),
       .D2(duc_q_reg[i]),
       .R(1'b0),
       .S(1'b0));
   end
   endgenerate
  
   // optional FPGA clk default disabled   //////////////////////////
   IBUFDS #(
      .DIFF_TERM("TRUE"),
      .IBUF_LOW_PWR("FALSE"),
      .IOSTANDARD("LVDS_25")
   ) IBUFDS_inst (
      .O(fpga_clk_ubuf),
      .I(FPGA_CLK_P),
      .IB(FPGA_CLK_N));
   
   BUFG BUFG_fpgaclk (
      .O(fpga_clk),
      .I(fpga_clk_ubuf));
  
   // PPS I/O SMA connector /////////////////////////////////////////
   assign PPS_IO = gpio_t[12] ? 1'bz : gpio_o[12];
   assign gpio_i[12] = PPS_IO;
  
   // ADC AD9634 ////////////////////////////////////////////////////
   IBUFDS #(
     .DIFF_TERM("TRUE"),
     .IBUF_LOW_PWR("FALSE"),
     .IOSTANDARD("LVDS_25")
   ) IBUFDS_ADC_OR_i (
     .O(adc_ovf),
     .I(ADC_OR_P),
     .IB(ADC_OR_N));
   assign gpio_i[40] = adc_ovf;

   generate
   for (i=0; i < 6; i=i+1) begin
     IBUFDS #(
       .DIFF_TERM("TRUE"),
       .IBUF_LOW_PWR("FALSE"),
       .IOSTANDARD("LVDS_25")
     ) IBUFDS_ADC_D0D1_i (
       .O(adc_ddr[i]),
       .I(ADC_D_P[i]),
       .IB(ADC_D_N[i]));
       
     IDDR #(
      .DDR_CLK_EDGE("OPPOSITE_EDGE"),
      .INIT_Q1(1'b0),
      .INIT_Q2(1'b0),
      .SRTYPE("SYNC")
     ) IDDR_inst (
      .Q1(adc_d1[i*2]),
      .Q2(adc_d1[i*2+1]),
      .C(adc_clk),
      .CE(1'b1),
      .D(adc_ddr[i]),
      .R(1'b0),
      .S(1'b0));
   end
   endgenerate
   assign adc_d[7:0] = adc_d1[7:0];
   assign adc_d[11:8] = ~adc_d1[11:8]; // P and N pins swapped

  // ADC SPI dummy
  assign ADC_SPI_CS = gpio_o[30];
  assign ADC_SPI_SCLK = gpio_o[31];
  assign ADC_SPI_SDIO = gpio_t[32] ? 1'bz : gpio_o[32];
  assign gpio_i[32] = ADC_SPI_SDIO;
					

  // System /////////////////////////////////////////////////////////////////////
  system_wrapper i_system_wrapper (
    .ddr_addr           (DDR_ADDR),
    .ddr_ba             (DDR_BA),
    .ddr_cas_n          (DDR_CAS_N),
    .ddr_ck_n           (DDR_CK_N),
    .ddr_ck_p           (DDR_CK_P),
    .ddr_cke            (DDR_CKE),
    .ddr_cs_n           (DDR_CS_N),
    .ddr_dm             (DDR_DM),
    .ddr_dq             (DDR_DQ),
    .ddr_dqs_n          (DDR_DQS_N),
    .ddr_dqs_p          (DDR_DQS_P),
    .ddr_odt            (DDR_ODT),
    .ddr_ras_n          (DDR_RAS_N),
    .ddr_reset_n        (DDR_RESET_N),
    .ddr_we_n           (DDR_WE_N),
    
    .sys_cpu_clk        (sys_cpu_clk),
    .sys_cpu_resetn     (sys_cpu_resetn),
    
    .fixed_io_ddr_vrn   (FIXED_IO_DDR_VRN),
    .fixed_io_ddr_vrp   (FIXED_IO_DDR_VRP),
    .fixed_io_mio       (FIXED_IO_MIO),
    .fixed_io_ps_clk    (FIXED_IO_PS_CLK),
    .fixed_io_ps_porb   (FIXED_IO_PS_PORB),
    .fixed_io_ps_srstb  (FIXED_IO_PS_SRSTB),
    
    .gpio_i             (gpio_i),
    .gpio_o             (gpio_o),
    .gpio_t             (gpio_t),
    
    .ext_pps_in         (ext_pps_in),
    .ext_pps_out        (ext_pps_out),
    .ext_pps_t          (ext_pps_t),
    .gps_pps_in         (GPS_PPS),
        
    .i2c1_scl_i         (i2c1_scl_i),
    .i2c1_scl_o         (i2c1_scl_o),
    .i2c1_scl_t         (i2c1_scl_t),
    .i2c1_sda_i         (i2c1_sda_i),
    .i2c1_sda_o         (i2c1_sda_o),
    .i2c1_sda_t         (i2c1_sda_t),
    
    .usb_oc_n           (USB_OCn),
    
    .spi0_clk           (spi0_clk),
    .spi0_csn_0         (spi0_csn[0]),
    .spi0_csn_1         (spi0_csn[1]),
    .spi0_csn_2         (spi0_csn[2]),
    .spi0_sdi           (spi0_miso),
    .spi0_sdo           (spi0_mosi),
    
    .duc_i              (duc_i),
    .duc_q              (duc_q),
    .duc_tx_en          (duc_tx_en),
    .pdclk_in           (DAC_PDCLK),
    .pdclk_out          (duc_pdclk),
    
    .adc_clk_in_p       (ADC_DCO_P),
    .adc_clk_in_n       (ADC_DCO_N),
    .adc_clk_out        (adc_clk),
    .adc_d              (adc_d),
    
    .UART_1_rxd         (GPS_TX),
    .UART_1_txd         (GPS_RX)
);

endmodule
