# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/badc0ded/DexterDABModulator/vitis/system_top/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/badc0ded/DexterDABModulator/vitis/system_top/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {system_top}\
-hw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu/system_top.xsa}\
-no-boot-bsp -out {/home/badc0ded/DexterDABModulator/vitis}

platform write
domain create -name {standalone_accel_microblaze_0} -display-name {standalone_accel_microblaze_0} -os {standalone} -proc {accel_microblaze_0} -runtime {cpp} -arch {32-bit} -support-app {hello_world}
platform generate -domains 
platform active {system_top}
platform generate -quick
bsp reload
bsp write
bsp reload
platform generate
bsp reload
bsp reload
platform generate -domains 
platform clean
platform generate
bsp reload
bsp reload
platform generate -domains 
platform config -updatehw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu/system_top.xsa}
platform generate -domains 
platform generate
platform active {system_top}
platform config -updatehw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/system_top.xsa}
platform generate
platform clean
platform generate
