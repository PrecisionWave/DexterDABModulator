# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/badc0ded/DexterDABModulator/vitis/dexter_apu/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/badc0ded/DexterDABModulator/vitis/dexter_apu/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {dexter_apu}\
-hw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu.xsa}\
-out {/home/badc0ded/DexterDABModulator/vitis}

platform write
domain create -name {standalone_accel_microblaze_0} -display-name {standalone_accel_microblaze_0} -os {standalone} -proc {accel_microblaze_0} -runtime {cpp} -arch {32-bit} -support-app {hello_world}
platform generate -domains 
platform active {dexter_apu}
domain active {zynq_fsbl}
domain active {standalone_accel_microblaze_0}
platform generate -quick
platform generate
platform config -remove-boot-bsp
platform write
bsp reload
bsp write
platform clean
platform generate
platform generate -domains standalone_accel_microblaze_0 
platform active {dexter_apu}
platform config -updatehw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu.xsa}
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {dexter_apu}
platform config -updatehw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu.xsa}
platform clean
platform generate
bsp reload
bsp reload
platform config -updatehw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu.xsa}
platform clean
platform generate
platform clean
platform clean
platform generate
bsp reload
platform active {dexter_apu}
platform config -updatehw {/home/badc0ded/DexterDABModulator/hdl_pcw/projects/dexter_apu/dexter_apu.xsa}
platform generate
platform clean
platform generate
bsp reload
