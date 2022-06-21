ARCH=(
aarch64-none-linux-gnueabi-static-i686.tar.gz
arm-none-linux-gnueabi-static-i686.tar.bz2
uboot-tools-i686-static.tar.gz
device-tree-compiler-i686-static.tar.gz
mkbootimage-e3796c3.tar.gz
)

mkdir -p bin
cd bin
for i in "${ARCH[@]}"; do
  curl -O "http://www.enclustra.com/binaries/enclustra-bsp/master/${i}"
done
for i in "${ARCH[@]}"; do
  tar -xf "${i}"
done
rm "${ARCH[@]}"
