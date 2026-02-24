
# FPGA resource usage

## Performance Microblaze with MMU @ 100 MHz

### Utilization Zynq 7020

| Name                                                 | Slice LUTs | Slice Registers | F7 Muxes  | F8 Muxes  | Slice      | LUT as Logic | LUT as Memory | Block RAM Tile | DSPs      |
| ---------------------------------------------------- | ---------- | --------------- | --------- | --------- | ---------- | ------------ | ------------- | -------------- | --------- |
| xlslice_tsc_47_32 (system_xlslice_1_0)               | 0          | 0               | 0         | 0         | 0          | 0            | 0             | 0              | 0         |
| xlslice_tsc_31_0 (system_xlslice_0_0)                | 0          | 0               | 0         | 0         | 0          | 0            | 0             | 0              | 0         |
| shared_mem (system_shared_mem_0)                     | 8          | 10              | 0         | 0         | 7          | 6            | 2             | 4              | 0         |
| shared_mailbox (system_shared_mailbox_0)             | 254        | 201             | 0         | 0         | 87         | 222          | 32            | 0              | 0         |
| microblaze_0 (system_microblaze_0_0)                 | 5775       | 6174            | 365       | 3         | 2330       | 5459         | 316           | 23             | 8         |
| mb_uartlite (system_mb_uartlite_0)                   | 92         | 110             | 1         | 0         | 35         | 82           | 10            | 0              | 0         |
| mb_shared_bram_ctrl (system_mb_shared_bram_ctrl_0)   | 189        | 200             | 0         | 0         | 79         | 189          | 0             | 0              | 0         |
| mb_rstgen (system_mb_rstgen_0)                       | 17         | 35              | 0         | 0         | 13         | 16           | 1             | 0              | 0         |
| mb_mdm (system_mb_mdm_0)                             | 90         | 143             | 0         | 0         | 40         | 83           | 7             | 0              | 0         |
| mb_int_concat (system_mb_int_concat_0)               | 0          | 0               | 0         | 0         | 0          | 0            | 0             | 0              | 0         |
| mb_axi_timer (system_mb_axi_timer_0)                 | 289        | 241             | 0         | 0         | 104        | 289          | 0             | 0              | 0         |
| mb_axi_per (system_mb_axi_per_0)                     | 179        | 120             | 0         | 0         | 101        | 179          | 0             | 0              | 0         |
| mb_axi_intc (system_mb_axi_intc_0)                   | 148        | 192             | 0         | 0         | 55         | 116          | 32            | 0              | 0         |
| mb_axi_gpio_tsc (system_mb_axi_gpio_tsc_0)           | 126        | 430             | 0         | 0         | 103        | 126          | 0             | 0              | 0         |
| mb_axi_cache (system_mb_axi_cache_0)                 | 2243       | 3185            | 0         | 0         | 1023       | 1947         | 296           | 0              | 0         |
| cpu_shared_bram_ctrl (system_cpu_shared_bram_ctrl_0) | 228        | 242             | 0         | 0         | 94         | 220          | 8             | 0              | 0         |
| cpu_mb_control (system_cpu_mb_control_0)             | 42         | 42              | 0         | 0         | 13         | 42           | 0             | 0              | 0         |
| cpu_cpu2apu_axi (system_cpu_cpu2apu_axi_0)           | 1733       | 2208            | 59        | 0         | 802        | 1577         | 156           | 0              | 0         |
| cpu_apuuart (system_cpu_apuuart_0)                   | 348        | 305             | 0         | 0         | 122        | 337          | 11            | 0              | 0         |
| apu_tsc (system_c_counter_binary_0_0)                | 1          | 48              | 0         | 0         | 12         | 1            | 0             | 0              | 0         |
| **Total accel (accel_imp_W1YUSQ)**                   | **11758**  | **13886**       | **425**   | **3**     | **4862**   | **10887**    | **871**       | **27**         | **8**     |
| Available xc7z020clg-400                             | 53200      | 106400          | 26600     | 13300     | 13300      | 53200        | 17400         | 140            | 220       |
| **accel (xc7z020clg-400 [%])**                       | **22.10%** | **13.05%**      | **1.60%** | **0.02%** | **36.56%** | **20.46%**   | **5.01%**     | **19.29%**     | **3.64%** |


### Encoding Performance
```
In=20.000000 Wait=8700 Enc=22475980 Send=10800
In=20.000000 Wait=8710 Enc=22587930 Send=10510
In=20.000000 Wait=8700 Enc=22902500 Send=11360
In=20.000000 Wait=8730 Enc=22515750 Send=17970
In=20.000000 Wait=8690 Enc=22623300 Send=10510
In=20.000000 Wait=8920 Enc=21877400 Send=9730
In=20.000000 Wait=8780 Enc=22092240 Send=10830
In=20.000000 Wait=9020 Enc=21765330 Send=10550
In=20.000000 Wait=8970 Enc=22283330 Send=11370
In=20.000000 Wait=8700 Enc=22199660 Send=18120
In=20.000000 Wait=8710 Enc=22073190 Send=10720

```