# Dexter BSP - Linux Kernel
PCW Linux kernel build specific information.  
For tools and common BSP information see [Dexter BSP](build_bsp.md).

## Building the Kernel
1. Fetch pcw-linux and enclustra-linux sources
```
cd <dexter git repo>/bsp_pcw/KERNEL
./fetch.sh
```

2. Build the Kernel
```
cd <dexter git repo>/bsp_pcw/KERNEL
./build-zx2.sh
```

This will generate the following files:
- `<dexter git repo>/bsp_pcw/KERNEL/out/zx2/dexter-min.dtb`
- `<dexter git repo>/bsp_pcw/KERNEL/out/zx2/dexter-max.dtb`
- `<dexter git repo>/bsp_pcw/KERNEL/out/zx2/uImage`

## Notes

### Kenel configuration
The Kernel configuration is done by the build-zx2.sh script.  
It creates the `pcw_defconfig` config file by appending
`<dexter git repo>/bsp_pcw/KERNEL/pcw-linux/arch/arm/configs/zynq_xcomm_adv7511_defconfig`
and every file in
`<dexter git repo>/bsp_pcw/KERNEL/kernel-config/zx2/*`.  
The pcw_defconfig file is located under
`<dexter git repo>/bsp_pcw/KERNEL/pcw-linux/arch/arm/configs/pcw_defconfig`.  
All Dexter specific kernel configuration can be found and changed in 
`<dexter git repo>/bsp_pcw/KERNEL/kernel-config/zx2/config`.

### Device trees
The Dexter device trees are located under
`<dexter git repo>/bsp_pcw/KERNEL/devicetrees/zx2`.  
All dts files in this directory are copied over to
`<dexter git repo>/bsp_pcw/KERNEL/pcw-linux/arch/arm/boot/dts`
by the build-zx2.sh script and compiled into dtb files.  
The dtb files are then copied to the 
`<dexter git repo>/bsp_pcw/KERNEL/out/zx2`
directory before the kernel gets built and can be therefore
fetched from the output directory before the kernel is ready.
