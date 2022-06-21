#!/bin/bash

cp ../../hdl_pcw/projects/dexter/dexter/dexter.runs/impl_1/system_top.bit in/zx2/fpga.bit.dexter7020 
cp ../../hdl_pcw/projects/dexter/dexter/dexter.runs/impl_2/system_top.bit in/zx2/fpga.bit.dexter7010

./build-zx2.sh

