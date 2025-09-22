ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"
BIN="$DIR/../bin"

# if no arg is given, we use the standard path to the UBOOT source
if [ $# -ne 1 ]; then
  TARG="$DIR/arm-trusted-firmware"
else
  TARG="${1%/}"
fi

if ! [ -d $TARG ]; then
  echo "target \"$TARG\" not found!"
  exit
fi


export PATH=$PATH:${BIN}/aarch64-none-linux-gnueabi/bin/
export PATH=$PATH:${BIN}/arm-none-linux-gnueabi-static/bin/
export ARCH=arm64
export CROSS_COMPILE=aarch64-none-linux-gnueabi-
export CROSS32CC=arm-none-linux-gnueabi-gcc

cd ${TARG}

make PLAT=zynqmp RESET_TO_BL31=1

cd ${ORG}
mkdir -p ${DIR}/out
cp ${TARG}/build/zynqmp/release/bl31/bl31.elf ${DIR}/out/
