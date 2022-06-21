# Linux IIO devices API description
The Dexter platform supports interfacing with Linux IIO devices.

# IIO Attributes
## dexter_dsp_tx
The TX DSP FPGA core contains two digital upconverters (DUC) and a PPS/GPSDO controller. The DUC is responsible for interpolation input IQ sample streams. They are interpolated by factor 80 from 2.048MSPS to 81.92MSPS and can be frequency shifted by +-33MHz.
The core is also responsible for the PPS jitter cleaning and the GPS disciplined OCXO oscillator (GPSDO) by providing clock counters and interfaces to the gpsdo.sh control loop script and the user layer by providing error metrics like frequency and timing error and GPSDO locked and PPS loss of signal indicators.

The DSP is clocked by 163.84MHz and the clock cycles counter values are reffered to this clock. 

| Attribute Name | Description | Value Range |
|----------------|-------------| ------------|
| buffer_underflows0 | counts number of missed samples on DUC0, that needs to be replaced with zeros | uint32_t |
| buffer_underflows1 | counts number of missed samples on DUC1, that needs to be replaced with zeros | uint32_t |
| dc0 | add a DC to DUC0 input, which leads to an umodulated carrier | 0..65535 |
| dc1 | add a DC to DUC1 input, which leads to an umodulated carrier | 0..65535 |
| dsp_version | DSP version YYMMDD | uint32_t |
| frequency0 | DUC0 frequency shift in Hz | -33000000 .. 33000000 |
| frequency1 | DUC1 frequency shift in Hz | -33000000 .. 33000000 |
| gain0 | DUC0 IQ input gain scaling, 65535 is 0dB | 0 .. 65535 |
| gain1 | DUC1 IQ input gain scaling, 65535 is 0dB | 0 .. 65535 |
| gpsdo_locked | READ ONLY, shows the status locked or unlocked of the GPSDO, the attribute is written by the gpsdo.sh control script | bool |
| pps_clk_error | READ ONLY,c lock cycles error of the OCXO clock within the last PPS period, used by the gpsdo.sh control script | int32_t |
| pps_clk_error_hz | READ ONLY, clock cycles frequency error in Herz of the OCXO clock within the last PPS period, written by the gpsdo.sh control script | int32_t |
| pps_clk_error_ns | READ ONLY, clock cycles time error in nanoseconds of the OCXO clock within the last PPS period, written by the gpsdo.sh control script  | int32_t |
| pps_clks | Number clock cycles since power up, updated on each PPS | uint64_t |
| pps_cnt | Number of PPS pulses since power up | uint32_t |
| pps_delay | PPS pulse delay for the PPS triggered streaming | 0 .. 16383999 |
