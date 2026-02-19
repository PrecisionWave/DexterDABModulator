# Debugging

## Get reserved memory for APU
The APU has full access to the PS memory. Downloading the program to anywhere that is not assigned to the
APU will most likely crash the PS.

```
$ dmesg | grep apu0
[    0.584941] 44050000.apu0_uart: ttyS0 at MMIO 0x44051000 (irq = 62, base_baud = 6250000) is a 16550A
[    0.598230] dexter_apu apu0: Dexter APU attached for device 0.
[    0.604102] dexter_apu apu0: Phys reg: 0x44000000 - 0x4402ffff
[    0.609959] dexter_apu apu0: Phys DMA: 0x16900000 - 0x169fffff
```

The `Phys DMA: 0x16900000 - 0x169fffff` contains the **dynamically** assigned memory region for the APU.
This address might change without notice on every boot.

The original `lscript.ld` has the following entry:
```
MEMORY
{
   sram : ORIGIN = 0x20000050, LENGTH = 0x3FB0
   ddr  : ORIGIN = 0x100000, LENGTH = 0x1FF00000
}
```

Update the `lscript.ld` to use the correct address and length:
```
MEMORY
{
   sram : ORIGIN = 0x20000050, LENGTH = 0x3FB0
   ddr  : ORIGIN = 0x16900000, LENGTH = 1048576
}
```

This way the jtag loader will load the ddr content to the reserved location in DDR and won't crash the PS.

## OpenOCD (JTAG)

Access to the debug port is possible via EMIO GPIO pins from the PS.

Zynq (E)MIO GPIO start at pin 906.
The 64 EMIOs are after the 54 MIO pins.

| Signal | Connection    | Calculation   | Linux GPIO number |
| ------ | ------------- | ------------- | ----------------- |
| tck    | emio_gpio[60] | 906 + 54 + 60 | 1020              |
| tms    | emio_gpio[61] | 906 + 54 + 61 | 1021              |
| tdi    | emio_gpio[62] | 906 + 54 + 62 | 1022              |
| tdo    | emio_gpio[0]  | 906 + 54 + 0  | 960               |


### Startup seqeunce for Dexter
#### Step 1: Release APU from sleep
Dexter:
```
$ ./load_apu -f hello_exc.elf
```
```
Page size: 4096 bytes
Assert APU Reset
Loading file hello_exc.elf
Zeroing memory...
  SRAM     0x20000000 - 20003fff
  DDR      0x16900000 - 16cfffff
Loading program...
Downloading 7276 bytes to DDR @ 0x16900000
Filling remaining 50028 bytes with 0 (DDR @ 0x16901c6c)
Downloading 842 bytes to SRAM @ 0x20000000
Relocating...
Successfully applied 1320 relocations
Release APU Reset
```

#### rv_gpio.cfg
```tcl
# openocd config for dexter_apu_vex
# CPU access to JTAG via EMIO GPIO pins

adapter driver sysfsgpio

bindto 0.0.0.0

# Each of the JTAG lines need a gpio number set: tck tms tdi tdo
# Zynq MIO GPIO start at pin 906
# The EMIO are after the 54 MIO
# tck: emio_gpio[60] 906 + 54 + 60  = 1020
# tms: emio_gpio[61] 906 + 54 + 61  = 1021
# tdi: emio_gpio[62] 906 + 54 + 62  = 1022
# tdo: emio_gpio[0]  906 + 54 + 0   = 960

sysfsgpio jtag_nums 1020 1021 1022 960

transport select jtag

set _CHIPNAME riscv
set _TARGETNAME $_CHIPNAME.cpu
jtag newtap $_CHIPNAME tap -irlen 5
target create $_TARGETNAME riscv -chain-position $_CHIPNAME.tap

init
```

#### Step 2: Start openocd
Dexter:
```
$ openocd -f rv_gpio.cfg
```
```
Open On-Chip Debugger 0.12.0+dev-02419-g129e9d300 (2023-01-12-15:46)
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
Info : SysfsGPIO JTAG/SWD bitbang driver
Info : Note: The adapter "sysfsgpio" doesn't support configurable speed
Info : JTAG tap: riscv.tap tap/device found: 0x10002fff (mfg: 0x7ff (<invalid>), part: 0x0002, ver: 0x1)
Info : [riscv.cpu.0] datacount=1 progbufsize=2
Info : [riscv.cpu.0] S?aia detected without IMSIC
Info : [riscv.cpu.0] Examined RISC-V core
Info : [riscv.cpu.0]  XLEN=32, misa=0x40001105
Info : [riscv.cpu.0] Examination succeed
Info : [riscv.cpu.0] starting gdb server on 3333
Info : Listening on port 3333 for gdb connections
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
```

#### Step 3: Start gdb
Host:
```
$ riscv-none-elf-gdb hello_exc.elf
```
```
GNU gdb (xPack GNU RISC-V Embedded GCC x86_64) 16.3
Copyright (C) 2024 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-pc-linux-gnu --target=riscv-none-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from hello_exc.elf...
(No debugging symbols found in hello_exc.elf)
(gdb) 
```
```
(gdb) target extended-remote 192.168.1.146:3333
```
```
Remote debugging using 192.168.1.146:3333
0x20000320 in trap_handler ()
(gdb) 
```

#### Step 4: Debug
Host:
```
(gdb) info registers
```
```
ra             0x0      0x0
sp             0x1690dfd8       0x1690dfd8
gp             0x16902428       0x16902428
tp             0x0      0x0
t0             0x1      1
t1             0x0      0
t2             0x0      0
fp             0x0      0x0
s1             0x0      0
a0             0x16901cb0       378543280
a1             0x1      1
a2             0x1      1
a3             0x169016b3       378541747
a4             0xa      10
a5             0x0      0
a6             0x0      0
a7             0x0      0
s2             0x0      0
s3             0x0      0
s4             0x0      0
s5             0x0      0
s6             0x0      0
s7             0x0      0
s8             0x0      0
s9             0x0      0
s10            0x0      0
s11            0x0      0
t3             0x0      0
t4             0x0      0
t5             0x0      0
t6             0x0      0
pc             0x20000320       0x20000320 <trap_handler+644>
(gdb) 
```


