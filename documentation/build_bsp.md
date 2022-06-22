# Dexter BSP

The Dexter BSP contains information how to build each file required to boot
the Enclustra Mars ZX2 FPGA module on the Dexter from sd-card.

## Overview

0. [ADI Kuiper Linux as Base](kuiper_sdcard.md)
1. [fpga.bit - Build the FPGA Bitstream](build_hdl.md)
2. [fsbl.elf - Build the First Stage Boot Loader](build_fsbl.md)
3. [u-boot.elf and uboot.scr - Build Das U-Boot bootloader](build_bsp_uboot.md)
4. [boot.bin - Build combined zynq PS/PL boot image](build_bsp_boot_bin.md)
5. [uImage and devicetree.dtb - Build the Kernel and Device tree](build_bsp_kernel.md)
6. [What to copy to the sd-card's BOOT parition](build_bsp_sdboot.md)

## Flow
### Base system
```mermaid
flowchart TD
    KUIPER["ADI Kuiper Linux"] -->|Download| DEV_PC
    DEV_PC["Developer's PC<br>fa:fa-file 2021-07-28-ADI-Kuiper-full.img"] -->|Flash onto sd-card| SDCARD
    
    subgraph SDCARD [Bootable SD-Card for ADI Dev Kits]
      td1("Boot partition<br>mmcblk0p1")
      td2("Root parition<br>mmcblk0p2")
    end
```

### Dexter specific
```mermaid
flowchart TD
    DSP_SRC[Dexter DSP source<br>fa:fa-wrench Matlab/Simulink] -->|Generate| DSP_LIB
    DSP_LIB[Dexter DSP library] -->|Use IP| PCW_HDL
    ADI_HDL[ADI HDL source<br>git:hdl_adi] -->|Build Library| ADI_HDL_LIB
    ADI_HDL_LIB[ADI HDL library] -->|Use IP| PCW_HDL
    PCW_HDL[Dexter HDL Project<br>fa:fa-wrench Vivado] -->|"Generate Bitstream"| FPGA_BIT
    PCW_HDL -->|"Export Hardware"| XSA
    FPGA_BIT["FPGA Bitstream <br>fa:fa-file system_top.bit"] -->|Copy to BSP and rename| BSP_BIT
    BSP_BIT["FPGA Bitstream <br>fa:fa-file fpga.bit.dexter70xx"] --> BSP_BOOT

    XSA["Hardware description<br>fa:fa-file system_top.xsa"] -->|Generate Application Project| VITIS
    VITIS[Application Project<br>fa:fa-wrench Vitis] -->|Build| FSBL
    FSBL["First Stage Boot Loader<br>fa:fa-file fsbl.elf"] -->|Copy to BSP| FSBL_IN
    FSBL_IN["First Stage Boot Loader<br>fa:fa-file fsbl.elf.dexter70xx"] --> BSP_BOOT

    UBOOT[Das U-Boot source<br>git:Xilinx/u-boot-xlnx<br>git:Enclustra/xilinx-uboot] -->|Fetch and merge| UBOOT_BUILD
    UBOOT_BUILD[Build<br>fa:fa-wrench build-zx2.sh] -->|Build| UBOOT_OUT
    UBOOT_OUT[Das U-Boot<br>fa:fa-file u-boot.elf<br>fa:fa-file uboot.scr] -->|Copy to BSP| UBOOT_IN
    UBOOT_IN[Das U-Boot<br>fa:fa-file u-boot.elf<br>fa:fa-file uboot.scr] --> BSP_BOOT

    BSP_BOOT[BOOT.BIN generation<br>fa:fa-wrench build-zx2.sh] --> BOOT_BIN
    BOOT_BIN[Boot Image<br>fa:fa-file BOOT.bin.dexter70xx<br>fa:fa-file u-boot.scr] -->|Copy to sd-card and rename| SDCARD_BOOT

    KERNEL_PCW[Kernel sources<br>git:pcw-linux] -->|Fetch| KERNEL_BUILD
    KERNEL_ENC[Kernel sources<br>git:enclustra-linux] -->|Fetch and copy| KERNEL_BUILD
    DTS[Device trees<br>fa:fa-file dexter-min.dts<br>fa:fa-file dexter-max.dts] -->|Copy| KERNEL_BUILD
    CFG_BASE[Kernel base configuration<br>fa:fa-file zynq_xcomm_adv7511_defconfig<br>fa:fa-file kernel-config/zx2/config] -->|Merge| KERNEL_BUILD
    KERNEL_BUILD[Kernel Build<br>fa:fa-wrench build-zx2.sh] --> KERNEL_BIN
    KERNEL_BIN["Kernel Binaries<br>fa:fa-file uImage<br>fa:fa-file dexter-min.dtb<br>fa:fa-file dexter-max.dtb"] -->|Copy to sd-card and rename| SDCARD_KERNEL

    subgraph Bootable SD-Card for Dexter
    SDCARD_KUIPER["Root parition<br>ADI Kuiper<br>2019_R2 Release"]
    SDCARD_BOOT["Boot partition on sd-card<br>fa:fa-file BOOT.bin<br>fa:fa-file u-boot.scr"]
    SDCARD_KERNEL["Boot partition on sd-card<br>fa:fa-file uImage<br>fa:fa-file devictree.dtb"]
    end
```

## Tools

Tested combination:  
- Ubuntu 18.04.6 LTS x86-64

Downloaded Tools:  
| sha1sum                                  | Package                                      | Version                                   |
| ---------------------------------------- | -------------------------------------------- | ----------------------------------------- |
| bf69cb2eb6feaa90ab61db311cc77143efcd0180 | arm-none-linux-gnueabi-static-i686.tar.bz2   | arm-none-linux-gnueabi-gcc (GCC) 7.2.0    |
| d96d8f2aa6218bc07fa40aff07654689ed85713e | device-tree-compiler-i686-static.tar.gz      | DTC 1.4.1-g8b927bf3-dirty                 |
| cee1d3d7bfeb2bb4e2890e62208a2168838f120f | mkbootimage-e3796c3.tar.gz                   | mkbootimage 2.2-e3796c3                   |
| fe57f1f99c6146f6018fdc0e219c375a03721e4d | uboot-tools-i686-static.tar.gz               | mkimage version 2015.01-g898bf01-dirty    |

## One-time preparation

### General compilers and Tools

1. Install build dependencies  
`sudo apt install -y autoconf build-essential libc6-i386 libssl-dev flex bison curl`

2. Fetch the build tools  
```
cd <dexter git repo>/bsp_pcw
./get-env.sh
```

3. Fetch pcw-linux and enclustra-linux sources
```
cd <dexter git repo>/bsp_pcw/KERNEL
./fetch.sh
```
