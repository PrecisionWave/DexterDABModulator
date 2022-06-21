ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"

SOMS=(${DIR}/out/*/)

for src in "${SOMS[@]}"; do
  trg="${src##*/out/}"
  trg=${DIR}/../BOOT/in/${trg}
  cp -v $src/u-boot.elf $trg
  cp -v $src/uboot.scr $trg
done
