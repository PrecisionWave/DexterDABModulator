# Dexter BSP - SD-Card BOOT
What to put on the SD-Card BOOT psrtition to boot the Enclustra ZX2 Module.  
For tools and common BSP information see [Dexter BSP](build_bsp.md).  

## Dependencies
1. [boot.bin - Build combined zynq PS/PL boot image](build_bsp_boot_bin.md)
2. [uImage and devicetree.dtb - Build the Kernel and Device tree](build_bsp_kernel.md)

## Preparing the BOOT partition

To boot the module the following files need to be present in the root of the BOOT partition:  
- boot.bin
- uboot.scr
- devicetree.dtb
- uImage

### uboot.scr
Copy `<dexter git repo>/bsp_pcw/BOOT/out/zx2/uboot.scr` to the root of the BOOT partition.

### uImage
Copy `<dexter git repo>/bsp_pcw/KERNEL/out/zx2/uImage` to the root of the BOOT partition.

### devicetree.dtb
Copy either `<dexter git repo>/bsp_pcw/KERNEL/out/zx2/dexter-min.dtb`
or `<dexter git repo>/bsp_pcw/KERNEL/out/zx2/dexter-max.dtb`
to the root of the BOOT partition and rename the file to `devicetree.bin`.
Which file to copy over depends on the PCB assembly variant in your device.

### boot.bin
Copy either `<dexter git repo>/bsp_pcw/BOOT/out/zx2/BOOT.bin.dexter7020`
or `<dexter git repo>/bsp_pcw/BOOT/out/zx2/BOOT.bin.dexter7010`
to the root of the BOOT partition and rename the file to `boot.bin`.
Which file to copy over depends on the Enclustra FPGA module used in your device.
