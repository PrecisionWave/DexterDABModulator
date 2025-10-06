# Microblaze Application Processor Unit for Dexter

## Startup / Reset
The Microblaze CPU is configured to start suspended. This means it will immediately halt after reset until it is woken
up by one of the wake-up pins.
The wake-up pins are exclusively controlled from the Zynq CPU over the cpu_mb_control GPIO block.
The reset pin is connected to a reset generator which in turn is controlled by various reset sources.
One of the reset sources is also connected to the Zynq CPU over the cpu_mb_control GPIO block.

## Program download
To download a program, hold the Microblaze in reset until ready to launch, download the program and then release the
reset and start the execution via the wake-up control.

## Loader application
The `load_apu` tool can load a Microblaze binary image to the block ram and start it.
The projects contains the mbox companion app for the Zynq and the Microblaze.

## Memory Maps
### Microblaze
| Address       | Size | Peripheral           |
| ------------- | ---- | -------------------- |
| `0x0000 0000` | 512M | Zynq main memory     |
| `0x2000 0000` | 16k  | Shared block ram     |
| `0x4060 0000` | 64k  | uartlite to Zynq     |
| `0x4120 0000` | 64k  | interrupt controller |
| `0x4360 0000` | 64k  | Shared mailbox       |

### Zynq
| Address       | Size | Peripheral         |
| ------------- | ---- | ------------------ |
| `0x0000 0000` | 512M | Zynq main memory   |
| `0x4400 0000` | 16k  | Shared block ram   |
| `0x4401 0000` | 64k  | Control GPIO       |
| `0x4402 0000` | 64k  | Shared mailbox     |
| `0x4403 0000` | 64k  | AXI DMA MB -> Zynq |
| `0x4404 0000` | 64k  | AXI DMA Zynq -> MB |
| `0x4405 0000` | 64k  | uartlite to MB     |

## Vectors
C_BASE_ADDRESS in the CPU configuration is set to `0x2000 0000` which in turn puts all vectors in the block ram.

## Demos
- Mailbox Demo [Mailbox Demo](./mbox.md)
- Relocation test [Relocation test](./reloc_test.md)
- Stream test [Stream test](./stream_test.md)

## System overview
![apu_v2](./apu_v2.png)
