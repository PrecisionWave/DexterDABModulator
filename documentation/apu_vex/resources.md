
# FPGA resource usage

## VexiiRiscv @ 100 MHz

### Utilization Zynq 7020

| Name                                                 | Slice LUTs | Slice Registers | F7 Muxes  | F8 Muxes  | Slice      | LUT as Logic | LUT as Memory | Block RAM Tile | DSPs      |
| ---------------------------------------------------- | ---------- | --------------- | --------- | --------- | ---------- | ------------ | ------------- | -------------- | --------- |
| sys_cpu2apu_axi (system_cpu_cpu2apu_axi_0)           | 1732       | 2208            | 59        | 0         | 768        | 1576         | 156           | 0              | 0         |
| shared_mem (system_shared_mem_0)                     | 8          | 10              | 0         | 0         | 7          | 6            | 2             | 4              | 0         |
| shared_mailbox (system_shared_mailbox_0)             | 253        | 200             | 0         | 0         | 88         | 221          | 32            | 0              | 0         |
| mb_rstgen (system_mb_rstgen_0)                       | 17         | 35              | 0         | 0         | 11         | 16           | 1             | 0              | 0         |
| cpu_shared_bram_ctrl (system_cpu_shared_bram_ctrl_0) | 229        | 242             | 0         | 0         | 88         | 221          | 8             | 0              | 0         |
| cpu_mb_control (system_cpu_mb_control_0)             | 42         | 42              | 0         | 0         | 16         | 42           | 0             | 0              | 0         |
| cpu_apuuart (system_cpu_apuuart_0)                   | 348        | 305             | 0         | 0         | 122        | 337          | 11            | 0              | 0         |
| apu_uartlite (system_mb_uartlite_0)                  | 89         | 107             | 1         | 0         | 32         | 79           | 10            | 0              | 0         |
| apu_tsc (system_c_counter_binary_0_0)                | 1          | 64              | 0         | 0         | 16         | 1            | 0             | 0              | 0         |
| apu_shared_bram_ctrl (system_mb_shared_bram_ctrl_0)  | 261        | 213             | 0         | 0         | 104        | 261          | 0             | 0              | 0         |
| apu_axi_per (system_mb_axi_per_0)                    | 83         | 115             | 0         | 0         | 40         | 83           | 0             | 0              | 0         |
| apu_axi_cache (system_mb_axi_cache_0)                | 4869       | 6815            | 16        | 0         | 1981       | 3943         | 926           | 10.5           | 0         |
| VexiiRiscv_wrapper_0 (system_VexiiRiscv_wrapper_0_0) | 8152       | 5866            | 69        | 0         | 2511       | 8033         | 119           | 27             | 4         |
| **Total accel (accel_imp_W1YUSQ)**                   | **16081**  | **16222**       | **145**   | **0**     | **5586**   | **14816**    | **1265**      | **41.5**       | **4**     |
| Available xc7z020clg-400                             | 53200      | 106400          | 26600     | 13300     | 13300      | 53200        | 17400         | 140            | 220       |
| **accel (xc7z020clg-400 [%])**                       | **30.23%** | **15.25%**      | **0.55%** | **0.00%** | **42.00%** | **27.85%**   | **7.27%**     | **29.64%**     | **1.82%** |


### Encoding Performance
```
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
In=20.000000 Wait=0 Enc=15 Send=0
In=20.000000 Wait=0 Enc=14 Send=0
In=20.000000 Wait=0 Enc=13 Send=0
```
