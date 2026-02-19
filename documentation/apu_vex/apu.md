# VexiiRiscv Riscv-V Application Processor Unit for Dexter

Custom VexiiRiscv core built from [VexiiRiscv](https://github.com/SpinalHDL/VexiiRiscv).

## System overview
![s](./diagrams/apu_v5.png)

Implementation details:
- [Block Design](./apu_v5.pdf)

### VexiiRiscv core

```bash
sbt "Test/runMain vexiiriscv.Generate \
    --xlen=32 \
    --with-rvm \
    --with-rvc \
    --with-rva \
    --with-rvZb \
    --with-rvZba \
    --with-rvZbb \
    --with-rvZbc \
    --with-rvZbs \
    --with-btb --with-gshare --with-ras \
    --mmu-sync-read \
    --fetch-l1 --fetch-l1-ways=2 \
    --fetch-axi4 \
    --lsu-l1 --lsu-l1-ways=2 --lsu-l1-axi4 \
    --lsu-axi4 \
    --region base=00000000,size=40000000,main=1,exe=1 \
    --region base=40000000,size=10000000,main=0,exe=0 \
    --reset-vector 0x20000000 \
    --debug-jtag-tap \
    --pmp-size 16 \
    --with-rdtime \
    --max-ipc \
    "
```

#### Configuration
RV32IMAC Zicsr

| Extension | Description                                                                                   |
| --------- | --------------------------------------------------------------------------------------------- |
| RV32I     | Base integer instruction set, 32-bit                                                          |
| M         | Integer mul/div                                                                               |
| A         | Atomics                                                                                       |
| C         | Compressed instructions (16-bit)                                                              |
| Zicsr     | Control and status register support                                                           |
| B         | The B standard extension comprises instructions provided by the Zba, Zbb, and Zbs extensions. |
| Zba       | Address generation                                                                            |
| Zbb       | Basic bit-manipulation                                                                        |
| Zbc       | Carry-less multiplication                                                                     |
| Zbs       | Single-bit instructions                                                                       |

#### General settings
    --xlen=32
        xlen = 32
    --with-rvm
        withMul = true
        withDiv = true
    --with-rvc
        withRvc = true
        withAlignerBuffer = true
    --with-rva
        withRva = true
    --with-rvZb
        withRvZba = true
        withRvZbb = true
        withRvZbc = true
        withRvZbs = true
    --with-rvZba
        withRvZba = true
    --with-rvZbb
        withRvZbb = true
    --with-rvZbc
        withRvZbc = true
    --with-rvZbs
        withRvZbs = true

#### Branch prediction
    --with-btb
        withBtb = true
    --with-gshare
        withGShare = true
    --with-ras
        withRas = true

#### Maybe remove
    --mmu-sync-read

#### Instruction cache
L1I with AXI4 bus.
4KB per way by default.

    --fetch-l1
        fetchL1Enable = true
    --fetch-l1-ways=2
        fetchL1Ways = 2
    --fetch-axi4
        fetchBus = FetchBusEnum.axi4

#### Data cache
L1D with AXI4 bus.
4KB per way by default.

    --lsu-l1
        lsuL1Enable = true
    --lsu-l1-ways=2
        lsuL1Ways = 2
    --lsu-l1-axi4
        lsuBus = LsuBusEnum.axi4

#### Peripheral bus
AXI4 bus peripherals

    --lsu-axi4

#### Memory regions
| From       | To         | Region      |
| ---------- | ---------- | ----------- |
| 0x00000000 | 0x1fffffff | DDR         |
| 0x20000000 | 0x20000fff | SRAM        |
| 0x40000000 | 0x4fffffff | Peripherals |

    --region base=00000000,size=40000000,main=1,exe=1
    --region base=40000000,size=10000000,main=0,exe=0

#### Reset vector
Reset vector at start of SRAM

    --reset-vector 0x20000000

#### JTAG
Add JTAG signals for debugger

    --debug-jtag-tap

#### Memory protection
Basic physical memory protection support with 16 entries (not MMU).

    --pmp-size 16

#### rdtime support
Support rdtime instruction to read a high accuracy timer.

    --with-rdtime

#### Max IPC
The max ipc enables a lot of things.

    --max-ipc
        withBtb = true                  # already enabled
        withGShare = true               # already enabled
        withRas = true                  # already enabled
        allowBypassFrom = 0
        divRadix = 4
        withLateAlu = true
        lsuMemDataWidthMin = 64         # 64-Bit Lsu Bus
        lsuL1Sets = 64
        lsuL1Ways = 4                   # from 2 to 4
        lsuL1RefillCount = 8
        lsuL1WritebackCount = 8
        lsuStoreBufferSlots = 4
        lsuStoreBufferOps = 32
        withLsuBypass = true
        lsuSoftwarePrefetch = true
        lsuHardwarePrefetch = "rpt"

### Interfaces
#### UART
The UART is the simplest communication interface in the system.
It's a simple uartlite on the APU side and an 16550 serial port on the CPU side.
It runs with a fixed bad rate of 115200 and is suitable for debug output.

#### Mailbox
The mailbox is a more elaborate way for data exchange. It is intended for synchronization of two independent running
systems. It is basically two FIFOs with control logic on either side.

#### Timer
An AXI timer with interrupt was added for timing purposes.

#### Timestamp counter
An 64-Bit free running incrementing counter that runs at cpu clock and is attached to rdtime port.

#### Shared Memory
There are two kind of shared memory intended for program loading and execution available;
- The SRAM is intended to be used exclusively by the APU. It is owned by the APU.
- The DDR interface is intended for bigger programs. It is owned by the CPU.

##### SRAM
All vectors from the APU are located in the SRAM. It is 16kB in size and is just big enough for system debugging.

##### Zynq DDR
The Zynq main memory can be used for program execution. To use this memory by the APU a little help of the CPU and
operating system is required. The APU can basically access all the CPU main memory. Synchronization and cache coherency
is an art of its own. It is recommended to only use it for program execution and not data transfer.

## Toolchain


## Startup / Reset
The Microblaze CPU is configured to start suspended. This means it will immediately halt after reset until it is woken
up by one of the wake-up pins.
The wake-up pins are exclusively controlled from the Zynq CPU over the cpu_mb_control GPIO block.
The reset pin is connected to a reset generator which in turn is controlled by various reset sources.
One of the reset sources is also connected to the Zynq CPU over the cpu_mb_control GPIO block.

## Program download
To download a program, hold the APU in reset until ready to launch, download the program and then release the
reset.

Use the `load_apu` tool to:
- load and start binary images to SRAM 
- load, relocate and start ELF files to SRAM and DDR memory

## Memory Maps
All addresses listed below are physical addresses as seen by the device.

| CPU view      | APU view      | Size | Peripheral                 |
| ------------- | ------------- | ---- | -------------------------- |
| `0x0000 0000` | `0x0000 0000` | 512M | CPU DDR (Zynq main memory) |
| `0x4400 0000` | `0x2000 0000` | 16k  | APU SRAM                   |
| `0x4402 0000` | `0x4360 0000` | 64k  | Shared mailbox             |
| n/a           | `0x4060 0000` | 64k  | APU uartlite UART          |
| n/a           | `0x4120 0000` | 64k  | APU interrupt controller   |
| n/a           | `0x41C0 0000` | 64k  | AXI Timer                  |
| `0x4405 0000` | n/a           | 64k  | CPU 16550 UART             |
| `0x4401 0000` | n/a           | 64k  | Reset control GPIO         |

## PMP
| Index | From          | To            | Size  | L   | A     | X   | W   | R   | Description                           |
| ----- | ------------- | ------------- | ----- | --- | ----- | --- | --- | --- | ------------------------------------- |
| 0     | `0x0000 0000` | `0x000f ffff` | 1M    | y   | NAPOT |     |     |     | Null pointer protection               |
| 1     | `0x0010 0000` | `0x3fff ffff` | 1023M | y   | NAPOT | y   | y   | y   | DDR and SRAM                          |
| 2     | `0x4000 0000` | `0x4fff ffff` | 256M  | y   | NAPOT |     | y   | z   | Peripheral Range                      |
| 3     | `0x5000 0000` | `0xffff ffff` | 2816M | y   | NAPOT |     |     |     | Nulll pointer underflow and no memory |

[RISC-V Privileged Specification](https://docs.riscv.org/reference/isa/_attachments/riscv-privileged.pdf)

## Tipps and tricks

### Debug
- [Debugging tipps](./debug.md)

### Serial port
The 16550 serial port `/dev/ttyS0` in the APU design might get claimed by the getty process.

This can be checked via the ps command:
```
$ ps aux | grep ttyS0
root       280  0.0  0.8  12944  4068 ttyS0    Ss   04:17   0:00 /bin/login -p --
root       396  0.0  0.7   8580  3748 ttyS0    S+   04:17   0:00 -bash
root       710  0.0  0.1   4880   524 pts/1    S+   04:34   0:00 grep ttyS0
```

To disable the service at startup:
```
$ systemctl disable serial-getty@ttyS0.service
```

To disable the currently running service now:
```
$ systemctl stop serial-getty@ttyS0.service
```

Recheck:
```
$ ps aux | grep ttyS0
root       787  0.0  0.1   4880   508 pts/1    S+   04:39   0:00 grep ttyS0
```
