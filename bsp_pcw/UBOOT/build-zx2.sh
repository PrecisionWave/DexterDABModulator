ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"
BIN="$DIR/../bin"

source ${DIR}/settings.sh

# if no arg is given, we use the standard path to the UBOOT source
if [ $# -ne 1 ]; then
  TARG="$DIR/${UBOOT_DIR}"
else
  TARG="${1%/}"
fi

if ! [ -d $TARG ]; then
  echo "target \"$TARG\" not found!"
  exit
fi

# output directory
OUT=${DIR}/out/zx2


#######################
# prepare environment #
#######################
export PATH=$PATH:${BIN}/aarch64-none-linux-gnueabi/bin/
export PATH=$PATH:${BIN}/arm-none-linux-gnueabi-static/bin/
export PATH=$PATH:${BIN}/uboot-tools-i686-static/


#################################
# create/clean output directory #
#################################
if [ -e ${OUT} ]; then
  rm -r ${OUT}
fi
mkdir -p ${OUT}


#########################
# copy devicetree files #
#########################
# prepare arrays for all devicetree files to copy
DTS=(${DIR}/devicetrees/zx2/*.dts)
DTB=()
for dts in "${DTS[@]}"; do
  temp="${dts##*/}"
  temp="${temp%*.dts}.dtb"
  DTB+=("$temp")
done

if [ ${#DTS[@]} -gt 1 ]; then
  echo "ERROR: currently we cannot support more than one devicetree file per platform!"
  echo "keep only one of your files:"
  ls -1 ${DIR}/devicetrees/zx2/
  exit 1
fi

# copy all devicetree files in array
for dts in "${DTS[@]}"; do
  cp ${dts} ${TARG}/arch/arm/dts/
done


################
# build U-BOOT #
################
cd $TARG

# define toolchain
export ARCH=arm
export CROSS_COMPILE=arm-none-linux-gnueabi-

# clean uboot
if [ ${CLEAN_BUILD} == "true" ]; then
  make distclean
fi

# generate & apply uboot config
cat configs/xilinx_zynq_virt_defconfig configs/zynq_mercury_defconfig configs/enclustra_env_on_sd_zynq.appendix > configs/pcw_defconfig  # for newer versions >= v1.9 of u-boot
# cat configs/zynq_mercury_defconfig configs/enclustra_env_on_sd.appendix > configs/pcw_defconfig  # compatible with u-boot v1.8.2
cat ${DIR}/config/zx2/* >> configs/pcw_defconfig

make pcw_defconfig

# build uboot
make -j $(($(nproc)-1))

# copy uboot binary to output directory
cp ${TARG}/u-boot.elf ${OUT}


######################
# generate uboot.src #
######################
echo "setenv kernel_size 0x1e80000" > ${TARG}/bscripts/mmcboot-pcw
echo "setenv devicetree_loadaddr 0x8000000" >> ${TARG}/bscripts/mmcboot-pcw
echo "" >> ${TARG}/bscripts/mmcboot-pcw
cat ${TARG}/bscripts/mmcboot-rootfs >> ${TARG}/bscripts/mmcboot-pcw
mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Uboot mmc start script" -d ${TARG}/bscripts/mmcboot-pcw ${OUT}/uboot.scr


##############################
# generate uboot_ramdisk.scr #
##############################
mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Uboot mmc ramdisk start script" -d ${TARG}/bscripts/mmcboot-ramdisk ${OUT}/uboot_ramdisk.scr

cd ${ORG}
exit
