#!/bin/bash

cp ../../hdl_pcw/projects/dexter/dexter/dexter.runs/impl_1/system_top.bit in/zx2/fpga.bit.dexter7020 
cp ../../hdl_pcw/projects/dexter/dexter/dexter.runs/impl_2/system_top.bit in/zx2/fpga.bit.dexter7010
cp ../../hdl_pcw/projects/dexter_apu/dexter_apu/dexter_apu.runs/impl_1/system_top.bit in/zx2/fpga.bit.apu
cp ../../hdl_pcw/projects/dexter_apu_vex/dexter_apu_vex/dexter_apu_vex.runs/impl_1/system_top.bit in/zx2/fpga.bit.apu_vex

./build-zx2.sh

