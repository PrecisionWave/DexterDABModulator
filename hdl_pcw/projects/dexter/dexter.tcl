# ***************************************************************************
# ***************************************************************************
# Copyright 2022(c) PrecisionWave AG
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#     - Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     - Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     - Neither the name of PrecisionWave AG nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#     - The use of this software may or may not infringe the patent rights
#       of one or more patent holders.  This license does not release you
#       from the requirement that you obtain separate licenses from these
#       patent holders to use this software.
#     - Use of the software either in source or binary form, must be run
#       on or directly connected to an Analog Devices Inc. component.
#
# THIS SOFTWARE IS PROVIDED BY PRECISIONWAVE "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED.
#
# IN NO EVENT SHALL PRECISIONWAVE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
# RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ***************************************************************************
# ***************************************************************************
# ***************************************************************************
# ***************************************************************************
#*****************************************************************************************
#
# Tcl script for re-creating project
#
# Usage:
#   1. Open Vivado
#   2. Select Tools/Run tcl script...
#   3. Navigate and select this tcl file
#   4. Run and wait till project is created
#
# Module:
#   Enclustra Mars ZX2, Zynq Z7020
#   Module PN:  MA-ZX2-20-2I-D9
#   FPGA PN:    xc7z020clg400-2 
#
#*****************************************************************************************

# Set the project details
set pcw_proj_name "dexter"
set pcw_fpga_part xc7z020clg400-2 
set pcw_bd_script dexter_bd.tcl
set pcw_xdc_file  system_constr.xdc
set adi_library   ../../../hdl_adi/library
set pcw_library   ../../library
set pcw_hdl_files [list system_top.v]

# Use script file location as working directory
cd [file dirname [file normalize [info script]]]

# Create project
source ../pcw_create_project.tcl

puts "Project creation finished!"
