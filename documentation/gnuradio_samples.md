# Dexter GNU Radio Examples

## Tools

Tested combination:
- Ubuntu 18.04.6 LTS
- GNU Radio Companion 3.7.13.4

## Examples

| Example Design    | Description                                       |
| ----------------- | --------------------------------------------------|
| simple_sine_src   | Simple Sine Source on Tx DMA #0 (ad9957_tx0)      |
| simple_sine_src2  | Simple Sine Source on Tx DMA #1 (ad9957_tx1)      |
| iq_file_playout   | Stream IQ samples from disk                       |

## Simple Sine Source
simple_sine_src example running on Detxer Min.

1. Setting up center frequency on Dexter  
`iio_attr -d ad9957 center_frequency 205800000`

2. Start GNUradio companion app on your PC  
`gnuradio-companion`

3. Load simple_sine_src.grc  
![Gnuradio companion](pictures/simple_sine_src_example/gnuradio_companion.png)

4. Execute the flow graph  
   "Run"->"Execute" on menu or "play" button in tool bar.

5. Setup tones  
![Runtime settings](pictures/simple_sine_src_example/settings.png)

6. Observe RF output  
![RF Measurement](pictures/simple_sine_src_example/rf_output.png)
