# Dexter HDL

## Tools
The standard edition of Vivado ML is sufficient to build the HDL for both FPGA modules.  
Download here: [Vivado ML Edition - 2021.1](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2021-1.html)

Tested combination (Linux):
- Ubuntu 18.04.6 LTS
- Vivado v2021.1.1 (64-bit)

Tested combination (Windows):
- Windows 11 Pro 21H2
- Vivado v2021.1 (64-bit)

## One-time preparation
### PrecisionWave DSP
There is a separate document describing how to generate the DSP from source.
To use the pre-built DSP IPs:
1. Naviagte to hdl_pcw/library
2. Unzip PrecisionWave_AG_pcwlib_dexter_dsp_tx_v1_0.zip into sub directory

### Fetch dependencies

```
git submodule init
git submodule update --remote
```

optional to fix HEAD detahced in ADI HDL submodule:
```
cd hdl_adi
git checkout hdl_2019_r2
cd ..
```

### ADI IP components
The ADI HDL components are delivered in pure source form and need a compilation step to be used in Vivado.

#### Windows
Building under Windows is not recommended but possible.

1. Open an suitable development environment  
e.g.mingw64 from Vivado
```
C:\Xilinx\Vivado\2021.1\tps\win64\msys64\mingw64.exe
```

2. Add Vivado to PATH
```
export PATH=/C/Xilinx/Vivado/2021.1/bin:/C/Xilinx/Vivado/2021.1/lib/win64.o:$PATH
```

2. Navigate to the axi_dmac directory  
```
cd /<drive>
cd <git checkout directory>
cd hdl_adi/library/axi_dmac
```

3. Setup environment to skip version check  
```
export ADI_IGNORE_VERSION_CHECK=1
```

4. Build axi_dmac  
```
make
```

#### Linux
1. Open console
2. Source environment
```
source /tools/Xilinx/Vivado/2021.1/settings64.sh
```

3. Navigate to the axi_dmac directory
```
cd hdl_adi/library/axi_dmac
```

4. Setup environment to skip version check  
```
set ADI_IGNORE_VERSION_CHECK=1
```

5. Build axi_dmac  
```
make
```


## Buidling the HDL
1. Open Vivado GUI
2. Select Tools/Run tcl script
3. Naviagte to hdl_pcw/projects/dexter
4. Select dexter.tcl and wait until finished
5. Check for errors
6. Generate Bitstream

This will generate the following files:  
- for impl_1: `<dexter git repo>/hdl_pcw/projects/dexter/dexter/dexter.runs/impl_1/system_top.bit`  
- for impl_2: `<dexter git repo>/hdl_pcw/projects/dexter/dexter/dexter.runs/impl_2/system_top.bit`  
