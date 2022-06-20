#*****************************************************************************************
#
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
#
#*****************************************************************************************
#
#  pcw_create_project.tcl
#
#*****************************************************************************************
#
# Source this script with the following variables set accordingly:
#  set pcw_proj_name
#  set pcw_fpga_parts list (or pcw_fpga_part)
#  set pcw_bd_script
#  set pcw_xdc_file
#  set pcw_library
#  set pcw_hdl_files
#  set adi_library
#
#*****************************************************************************************

# Compatibility with older scripts
if {[info exists pcw_fpga_part]} {
    set pcw_fpga_parts [list $pcw_fpga_part]
}

if {![info exists adi_library]} {
    set adi_library $pcw_library
}

# Check parameters
if {![info exists pcw_proj_name]}  {puts "ERROR: pcw_proj_name not set!"; return false;}
if {![info exists pcw_fpga_parts]} {puts "ERROR: pcw_fpga_parts nor pcw_fpga_part set!"; return false;}
if {![info exists pcw_bd_script]}  {puts "ERROR: pcw_bd_script not set!"; return false;}
if {![info exists pcw_xdc_file]}   {puts "ERROR: pcw_xdc_file not set!"; return false;}
if {![info exists pcw_library]}    {puts "ERROR: pcw_library not set!"; return false;}
if {![info exists adi_library]}    {puts "ERROR: adi_library not set!"; return false;}
if {![info exists pcw_hdl_files]}  {puts "ERROR: pcw_hdl_files not set!"; return false;}

# Add ad_iobuf
lappend pcw_hdl_files $adi_library/common/ad_iobuf.v

# Check bd version
set f [open $pcw_bd_script r]
for {set i 0} {[gets $f line] >= 0} {incr i} {
  if {[string first "set scripts_vivado_version" $line] != -1} {
    puts $line
    eval $line
    if { [info exists scripts_vivado_version] == 1} {
      set current_vivado_version [version -short]

      if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
        puts "BD Version check failed: The BD script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado."
        return 1
      }
    }
  }
}

puts "Version check OK"

# Close current open project
catch close_project

# Check if project exists and create backup
set ts [clock format [clock seconds] -format %y-%m-%d_%H_%M_%S]
catch {file rename ${pcw_proj_name} "${pcw_proj_name}_backup_$ts" }

# Set the reference directory for source file relative paths (by default the value is script directory path)
set origin_dir "."

# Create project
set fpga_part [lindex $pcw_fpga_parts 0]
create_project ${pcw_proj_name} ./${pcw_proj_name} -part ${fpga_part}

# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# Set project properties
set obj [current_project]
set_property -name "default_lib" -value "xil_defaultlib" -objects $obj
set_property -name "dsa.accelerator_binary_content" -value "bitstream" -objects $obj
set_property -name "dsa.accelerator_binary_format" -value "xclbin2" -objects $obj
set_property -name "dsa.description" -value "Vivado generated DSA" -objects $obj
set_property -name "dsa.dr_bd_base_address" -value "0" -objects $obj
set_property -name "dsa.emu_dir" -value "emu" -objects $obj
set_property -name "dsa.flash_interface_type" -value "bpix16" -objects $obj
set_property -name "dsa.flash_offset_address" -value "0" -objects $obj
set_property -name "dsa.flash_size" -value "1024" -objects $obj
set_property -name "dsa.host_architecture" -value "x86_64" -objects $obj
set_property -name "dsa.host_interface" -value "pcie" -objects $obj
set_property -name "dsa.num_compute_units" -value "60" -objects $obj
set_property -name "dsa.platform_state" -value "pre_synth" -objects $obj
set_property -name "dsa.vendor" -value "xilinx" -objects $obj
set_property -name "dsa.version" -value "0.0" -objects $obj
set_property -name "enable_vhdl_2008" -value "1" -objects $obj
set_property -name "ip_cache_permissions" -value "read write" -objects $obj
set_property -name "ip_output_repo" -value "$proj_dir/${pcw_proj_name}.cache/ip" -objects $obj
set_property -name "mem.enable_memory_map_generation" -value "1" -objects $obj
set_property -name "part" -value "${fpga_part}" -objects $obj
set_property -name "sim.central_dir" -value "$proj_dir/${pcw_proj_name}.ip_user_files" -objects $obj
set_property -name "sim.ip.auto_export_scripts" -value "1" -objects $obj
set_property -name "simulator_language" -value "Mixed" -objects $obj
set_property -name "webtalk.activehdl_export_sim" -value "2" -objects $obj
set_property -name "webtalk.ies_export_sim" -value "2" -objects $obj
set_property -name "webtalk.modelsim_export_sim" -value "2" -objects $obj
set_property -name "webtalk.questa_export_sim" -value "2" -objects $obj
set_property -name "webtalk.riviera_export_sim" -value "2" -objects $obj
set_property -name "webtalk.vcs_export_sim" -value "2" -objects $obj
set_property -name "webtalk.xsim_export_sim" -value "2" -objects $obj
set_property -name "xpm_libraries" -value "XPM_CDC XPM_FIFO XPM_MEMORY" -objects $obj

# Create 'sources_1' fileset (if not found)
if {[string equal [get_filesets -quiet sources_1] ""]} {
  create_fileset -srcset sources_1
}

# Set IP repository paths
set obj [get_filesets sources_1]
set_property "ip_repo_paths" "[file normalize "$origin_dir/$pcw_library"] [file normalize "$origin_dir/$adi_library"]" $obj

# Rebuild user ip_repo's index before adding any source files
update_ip_catalog -rebuild

# Set 'sources_1' fileset object
set obj [get_filesets sources_1]

# Add local files from the original project (-no_copy_sources specified)
set files [list ]
foreach file $pcw_hdl_files {
    lappend files [file normalize "${origin_dir}/${file}" ]
}
puts $files
set added_files [add_files -fileset sources_1 $files]

# Set 'sources_1' fileset file properties for remote files
# None

# Set 'sources_1' fileset file properties for local files
# None

# Set 'sources_1' fileset properties
# None

# Create 'constrs_1' fileset (if not found)
if {[string equal [get_filesets -quiet constrs_1] ""]} {
  create_fileset -constrset constrs_1
}

# Set 'constrs_1' fileset object
set obj [get_filesets constrs_1]

# Add/Import constrs file and set constrs file properties
set file "[file normalize "$origin_dir/$pcw_xdc_file"]"
set file_added [add_files -norecurse -fileset $obj [list $file]]
set file "$pcw_xdc_file"
set file_obj [get_files -of_objects [get_filesets constrs_1] [list "*$file"]]
set_property -name "file_type" -value "XDC" -objects $file_obj

# Set 'constrs_1' fileset properties
set obj [get_filesets constrs_1]
set_property -name "target_constrs_file" -value "[get_files *$pcw_xdc_file]" -objects $obj
set_property -name "target_part" -value "${fpga_part}" -objects $obj
set_property -name "target_ucf" -value "[get_files *$pcw_xdc_file]" -objects $obj

# Create 'sim_1' fileset (if not found)
if {[string equal [get_filesets -quiet sim_1] ""]} {
  create_fileset -simset sim_1
}

# Set 'sim_1' fileset object
set obj [get_filesets sim_1]
# Empty (no sources present)

# Set 'sim_1' fileset properties
set obj [get_filesets sim_1]
set_property -name "top" -value "system_top" -objects $obj
set_property -name "top_lib" -value "xil_defaultlib" -objects $obj

# Set 'utils_1' fileset object
set obj [get_filesets utils_1]
# Empty (no sources present)

# Set 'utils_1' fileset properties
set obj [get_filesets utils_1]

# Adding sources referenced in BDs, if not already added

# Proc to create BD system
source ${pcw_bd_script}

set_property REGISTERED_WITH_MANAGER "1" [get_files system.bd ] 
set_property SYNTH_CHECKPOINT_MODE "Hierarchical" [get_files system.bd ] 

make_wrapper -files [get_files system.bd] -top
add_files -norecurse [file normalize "${origin_dir}/${pcw_proj_name}/${pcw_proj_name}.srcs/sources_1/bd/system/hdl/system_wrapper.v" ]
update_compile_order -fileset sources_1

for {set i 1} {$i < [llength $pcw_fpga_parts]} {incr i} {
    set fpga [lindex $pcw_fpga_parts $i]
    set n [expr "$i + 1"]
    create_run synth_$n -part $fpga -flow {Vivado Synthesis 2021}
    create_run impl_$n -part $fpga -parent_run synth_$n -flow {Vivado Implementation 2021}
}

