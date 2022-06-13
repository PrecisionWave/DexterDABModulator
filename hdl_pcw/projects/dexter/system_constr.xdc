# Enclustra MA-ZX2-20-2I-D9
# Zynq Z7020 xc7z020clg400-2

# general
#set_property CFGBVS VCCO [current_design]
#set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property BITSTREAM.CONFIG.OVERTEMPPOWERDOWN ENABLE [current_design]

# ----------------------------------------------------------------------------------
# Important! Do not remove this constraint!
# This property ensures that all unused pins are set to high impedance.
# If the constraint is removed, all unused pins have to be set to HiZ in the top level file.
set_property BITSTREAM.CONFIG.UNUSEDPIN PULLNONE [current_design]
# ----------------------------------------------------------------------------------

# DAC AD9957
set_property -dict {PACKAGE_PIN U18 IOSTANDARD LVCMOS33} [get_ports DAC_PDCLK]
set_property -dict {PACKAGE_PIN U14 IOSTANDARD LVCMOS33} [get_ports {DAC_D[0]}]
set_property -dict {PACKAGE_PIN U15 IOSTANDARD LVCMOS33} [get_ports {DAC_D[1]}]
set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS33} [get_ports {DAC_D[2]}]
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports {DAC_D[3]}]
set_property -dict {PACKAGE_PIN U13 IOSTANDARD LVCMOS33} [get_ports {DAC_D[4]}]
set_property -dict {PACKAGE_PIN V13 IOSTANDARD LVCMOS33} [get_ports {DAC_D[5]}]
set_property -dict {PACKAGE_PIN T11 IOSTANDARD LVCMOS33} [get_ports {DAC_D[6]}]
set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports {DAC_D[7]}]
set_property -dict {PACKAGE_PIN V12 IOSTANDARD LVCMOS33} [get_ports {DAC_D[8]}]
set_property -dict {PACKAGE_PIN W13 IOSTANDARD LVCMOS33} [get_ports {DAC_D[9]}]
set_property -dict {PACKAGE_PIN W14 IOSTANDARD LVCMOS33} [get_ports {DAC_D[10]}]
set_property -dict {PACKAGE_PIN Y14 IOSTANDARD LVCMOS33} [get_ports {DAC_D[11]}]
set_property -dict {PACKAGE_PIN P14 IOSTANDARD LVCMOS33} [get_ports {DAC_D[12]}]
set_property -dict {PACKAGE_PIN R14 IOSTANDARD LVCMOS33} [get_ports {DAC_D[13]}]
set_property -dict {PACKAGE_PIN T14 IOSTANDARD LVCMOS33} [get_ports {DAC_D[14]}]
set_property -dict {PACKAGE_PIN T15 IOSTANDARD LVCMOS33} [get_ports {DAC_D[15]}]
set_property -dict {PACKAGE_PIN T16 IOSTANDARD LVCMOS33} [get_ports {DAC_D[16]}]
set_property -dict {PACKAGE_PIN U17 IOSTANDARD LVCMOS33} [get_ports {DAC_D[17]}]
set_property -dict {PACKAGE_PIN P15 IOSTANDARD LVCMOS33} [get_ports DAC_TxEN]
set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports DAC_IO_RESET]
set_property -dict {PACKAGE_PIN P18 IOSTANDARD LVCMOS33} [get_ports DAC_MR]
set_property -dict {PACKAGE_PIN P16 IOSTANDARD LVCMOS33} [get_ports DAC_IO_UPDATE]

# ADC AD9634, on lane 4 and 5 P and N is swapped
set_property -dict {PACKAGE_PIN H16 IOSTANDARD LVDS_25} [get_ports ADC_DCO_P]
set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVDS_25} [get_ports ADC_DCO_N]
set_property -dict {PACKAGE_PIN G19 IOSTANDARD LVDS_25} [get_ports ADC_OR_P]
set_property -dict {PACKAGE_PIN G20 IOSTANDARD LVDS_25} [get_ports ADC_OR_N]
set_property -dict {PACKAGE_PIN J20 IOSTANDARD LVDS_25} [get_ports {ADC_D_P[0]}]
set_property -dict {PACKAGE_PIN H20 IOSTANDARD LVDS_25} [get_ports {ADC_D_N[0]}]
set_property -dict {PACKAGE_PIN J18 IOSTANDARD LVDS_25} [get_ports {ADC_D_P[1]}]
set_property -dict {PACKAGE_PIN H18 IOSTANDARD LVDS_25} [get_ports {ADC_D_N[1]}]
set_property -dict {PACKAGE_PIN K19 IOSTANDARD LVDS_25} [get_ports {ADC_D_P[2]}]
set_property -dict {PACKAGE_PIN J19 IOSTANDARD LVDS_25} [get_ports {ADC_D_N[2]}]
set_property -dict {PACKAGE_PIN L19 IOSTANDARD LVDS_25} [get_ports {ADC_D_P[3]}]
set_property -dict {PACKAGE_PIN L20 IOSTANDARD LVDS_25} [get_ports {ADC_D_N[3]}]
set_property -dict {PACKAGE_PIN N15 IOSTANDARD LVDS_25} [get_ports {ADC_D_P[4]}]
set_property -dict {PACKAGE_PIN N16 IOSTANDARD LVDS_25} [get_ports {ADC_D_N[4]}]
set_property -dict {PACKAGE_PIN M19 IOSTANDARD LVDS_25} [get_ports {ADC_D_P[5]}]
set_property -dict {PACKAGE_PIN M20 IOSTANDARD LVDS_25} [get_ports {ADC_D_N[5]}]

# ADC SPI
set_property -dict {PACKAGE_PIN K16   IOSTANDARD LVCMOS25  } [get_ports {ADC_SPI_SDIO}]
set_property -dict {PACKAGE_PIN J16   IOSTANDARD LVCMOS25  } [get_ports {ADC_SPI_SCLK}]
set_property -dict {PACKAGE_PIN G17   IOSTANDARD LVCMOS25  } [get_ports {ADC_SPI_CS}]

# I2C
set_property -dict {PACKAGE_PIN T20 IOSTANDARD LVCMOS33  } [get_ports {I2C_INT_N}]
set_property -dict {PACKAGE_PIN W20 IOSTANDARD LVCMOS33} [get_ports I2C_SCL]
set_property -dict {PACKAGE_PIN V20 IOSTANDARD LVCMOS33} [get_ports I2C_SDA]

# SPI
set_property -dict {PACKAGE_PIN R16 IOSTANDARD LVCMOS33} [get_ports SPI_MISO]
set_property -dict {PACKAGE_PIN W16 IOSTANDARD LVCMOS33} [get_ports SPI_MOSI]
set_property -dict {PACKAGE_PIN R17 IOSTANDARD LVCMOS33} [get_ports SPI_SCK]

# SPI CS
set_property -dict {PACKAGE_PIN N18 IOSTANDARD LVCMOS33} [get_ports nCS_AD9957]
set_property -dict {PACKAGE_PIN Y17 IOSTANDARD LVCMOS33} [get_ports nCS_LMK]
set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports nCS_AD9833]

# PLL
set_property -dict {PACKAGE_PIN Y18 IOSTANDARD LVCMOS33} [get_ports Status_LD]
set_property -dict {PACKAGE_PIN Y19 IOSTANDARD LVCMOS33} [get_ports Status_CLKin0]
set_property -dict {PACKAGE_PIN W18 IOSTANDARD LVCMOS33} [get_ports Status_CLKin1]

# DDS
set_property -dict {PACKAGE_PIN U20 IOSTANDARD LVCMOS33} [get_ports extRef_nOCXO]

# GPS
set_property -dict {PACKAGE_PIN V16 IOSTANDARD LVCMOS33} [get_ports PPS_IO]
set_property -dict {PACKAGE_PIN R18 IOSTANDARD LVCMOS33} [get_ports GPS_PPS]
set_property -dict {PACKAGE_PIN U12 IOSTANDARD LVCMOS33} [get_ports GPS_TX]
set_property -dict {PACKAGE_PIN T12 IOSTANDARD LVCMOS33} [get_ports GPS_RX]

# USB
set_property -dict {PACKAGE_PIN D18 IOSTANDARD LVCMOS25} [get_ports USB_OCn]

# Front
set_property -dict {PACKAGE_PIN L17 IOSTANDARD LVCMOS25} [get_ports Button_Front]
set_property -dict {PACKAGE_PIN F16 IOSTANDARD LVCMOS25} [get_ports ENn_LED_red]
set_property -dict {PACKAGE_PIN F17 IOSTANDARD LVCMOS25} [get_ports ENn_LED_green]

# Module LEDs
set_property -dict {PACKAGE_PIN R19 IOSTANDARD LVCMOS33} [get_ports {MODULE_LED[0]}]
set_property -dict {PACKAGE_PIN T19 IOSTANDARD LVCMOS33} [get_ports {MODULE_LED[1]}]
set_property -dict {PACKAGE_PIN G14 IOSTANDARD LVCMOS25} [get_ports {MODULE_LED[2]}]
set_property -dict {PACKAGE_PIN J15 IOSTANDARD LVCMOS25} [get_ports {MODULE_LED[3]}]

# GPIO
set_property -dict {PACKAGE_PIN P19   IOSTANDARD LVCMOS33  } [get_ports {GPIO5}]
set_property -dict {PACKAGE_PIN N20   IOSTANDARD LVCMOS33  } [get_ports {GPIO4}]
set_property -dict {PACKAGE_PIN P20   IOSTANDARD LVCMOS33  } [get_ports {GPIO3}]
set_property -dict {PACKAGE_PIN V15   IOSTANDARD LVCMOS33  } [get_ports {GPIO2}]
set_property -dict {PACKAGE_PIN W15   IOSTANDARD LVCMOS33  PULLUP TRUE  } [get_ports {SDA_GPIO1}]
set_property -dict {PACKAGE_PIN Y16   IOSTANDARD LVCMOS33  PULLUP TRUE  } [get_ports {SCL_GPIO0}]

# Clock
set_property -dict {PACKAGE_PIN K17   IOSTANDARD LVDS_25  DIFF_TERM TRUE } [get_ports {FPGA_CLK_P}]
set_property -dict {PACKAGE_PIN K18   IOSTANDARD LVDS_25  DIFF_TERM TRUE } [get_ports {FPGA_CLK_N}]


# Clock Frequencies
#create_clock -name ADC_CLK          -period  6.104      [get_ports ADC_DCO_P]
create_clock -name FPGA_CLK         -period  6.104      [get_ports FPGA_CLK_P]
#create_clock -name DAC_PDCLK_IN     -period  12.207     [get_ports DAC_PDCLK]