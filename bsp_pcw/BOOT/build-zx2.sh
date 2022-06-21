ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"
BIN="$DIR/../bin"
TMP="$DIR/tmp"

# input/output directory
IN=${DIR}/in/zx2
OUT=${DIR}/out/zx2


#################################
# create/clean output directory #
#################################
if [ -e ${OUT} ]; then
  rm -r ${OUT}
fi
mkdir -p ${OUT}


mkdir -p ${TMP}
cp ${IN}/* ${TMP}
cd ${TMP}
BITSTREAMS=$(ls -1 fpga.bit.*)
for BIT in ${BITSTREAMS[@]}; do
  TRG=${BIT##*.}
  FSBL=fsbl.elf.${TRG}

  echo -e "\n\033[0;32mbuilding BOOT.bin.${TRG}\033[0m"
  cp ${BIT} fpga.bit
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR: missing file ${BIT}, skipping build\033[0m"
    continue
  fi
  cp ${FSBL} fsbl.elf
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR: missing file ${FSBL}, skipping build\033[0m"
    continue
  fi
  ${BIN}/mkbootimage/mkbootimage boot.bif ${OUT}/BOOT.bin.${TRG}
done
cp uboot.scr ${OUT}
cd ${ORG}
rm -r ${TMP}
