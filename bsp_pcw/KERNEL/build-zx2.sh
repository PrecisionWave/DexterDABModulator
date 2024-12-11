#!/bin/bash

# caller path
ORG="$(pwd)"

# directory path
DIR="$ORG/$(dirname $0)"
echo "$DIR"
if [ "${DIR: -1}" = "." ]; then
  DIR="${DIR:0:-1}"
fi
DIR="${DIR%/}"

# toolchain path
BIN="$DIR/../bin"

# input/output directories
IN=${DIR}/in/zx2
OUT=${DIR}/out/zx2

# script usage
if [ $# -gt 1 ]; then
  echo "USAGE:   build.sh <linux-folder-path>"
  exit
fi

source ${DIR}/settings.sh


###############
# target path #
###############
if [ $# -lt 1 ]; then
  TARG="${DIR}/${KERNEL_DIR}"
else
  TARG="${1%/}"
fi
if ! [ -d $TARG ]; then
  echo "target \"$TARG\" not found!"
  exit
else
  echo "Note: building kernel \"$TARG\""
fi


##################
# enclustra path #
##################
if ! [ -d ${DIR}/enclustra-linux ]; then
  echo "\"${DIR}/enclustra-linux\" not found!"
  echo "please call \"./fetch.sh\" first"
  exit
else
  # checkout clean master branch of enclustra-linux
  git -C ${DIR}/enclustra-linux reset --hard
  git -C ${DIR}/enclustra-linux clean -fd
  git -C ${DIR}/enclustra-linux checkout master
fi


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


###############################
# copy adau1761 firmware file #
###############################
#cp ${DIR}/pcw_sources/adau1761.bin ${TARG}/firmware/


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

# copy all devicetree files in array
for dts in "${DTS[@]}"; do
  cp ${dts} ${TARG}/arch/arm/boot/dts/
done
cp ${DIR}/enclustra-linux/arch/arm/boot/dts/zynq_enclustra_* ${TARG}/arch/arm/boot/dts/
cp ${DIR}/enclustra-linux/arch/arm/boot/dts/zynq-enclustra-* ${TARG}/arch/arm/boot/dts/
cp ${DIR}/devicetrees/*.dtsi ${TARG}/arch/arm/boot/dts/


################################
# build devicetrees and kernel #
################################
cd ${TARG}

# define toolchain
export ARCH=arm
export CROSS_COMPILE=arm-none-linux-gnueabi-

# clean kernel
if [ ${CLEAN_BUILD} == "true" ]; then
  make distclean
fi

# generate & apply kernel config
cat arch/arm/configs/zynq_xcomm_adv7511_defconfig > arch/arm/configs/pcw_defconfig
cat ${DIR}/kernel-config/zx2/* >> arch/arm/configs/pcw_defconfig
make pcw_defconfig

# build devicetrees
for dtb in "${DTB[@]}"; do
  make ${dtb}
done

# copy devicetree blops to output directory
for dtb in "${DTB[@]}"; do
  cp ${TARG}/arch/arm/boot/dts/${dtb} ${OUT}
done

# build linux kernel
make -j $(($(nproc)-1)) uImage UIMAGE_LOADADDR=0x8000

# copy kernel image to output directory
cp ${TARG}/arch/arm/boot/uImage ${OUT}

cd ${ORG}
exit
