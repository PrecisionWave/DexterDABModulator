# Dexter BSP - Das U-Boot
Das U-Boot build specific information.  
For tools and common BSP information see [Dexter BSP](build_bsp.md).  
Usually Das U-Boot needs to be built only once.

## Build U-Boot
1. Fetch U-boot sources and dependencies
```
cd <dexter git repo>/bsp_pcw/UBOOT
./fetch.sh
```

2. Build U-Boot for the ZX2 Module
```
cd <dexter git repo>/bsp_pcw/UBOOT
./build-zx2.sh
```

This will generate the following files:
- `<dexter git repo>/bsp_pcw/UBOOT/out/zx2/u-boot.elf`
- `<dexter git repo>/bsp_pcw/UBOOT/out/zx2/uboot.scr`
- `<dexter git repo>/bsp_pcw/UBOOT/out/zx2/uboot_ramdisk.scr`

3. Copy resulting files to BOOT
```
cd <dexter git repo>/bsp_pcw/UBOOT
./copy-results.sh
```

This will generate the following files:
- `<dexter git repo>/bsp_pcw/BOOT/in/zx2/u-boot.elf`
- `<dexter git repo>/bsp_pcw/BOOT/in/zx2/uboot.scr`
