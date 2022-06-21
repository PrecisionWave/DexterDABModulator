The Dexter platform supports interfacing with Linux IIO devices.

# IIO Attributes
## dexter_dsp_tx
The TX DSP FPGA core is responsible for digital upconversion of the two input IQ sample streams. They are interpolated from 2.048MSPS to 81.92MSPS and can be moved within +-33MHz.
The core is also responsible for the PPS jitter cleaning and the GPS disciplined OCXO oscillator by providing clock counters and interfaces to the gpsdo.sh control loop script and the user layer by providing error metrics like frequency and timing error and GPSDO locked and PPS loss of signal indicators.

| Attribute Name | Description |
|----------------|-------------|
| buffer_underflows0 | counts number of missed samples, that needs to be replaced with zeros |
