ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"
BIN="$DIR/../bin"

source ${DIR}/settings.sh

#git clone https://github.com/Xilinx/u-boot-xlnx.git
git clone https://github.com/enclustra-bsp/xilinx-uboot $UBOOT_DIR
git -C $UBOOT_DIR checkout $UBOOT_BRANCH

exit  # the merge routine below is not necessary anymore (11.2021)

cd $UBOOT_DIR
git remote add xlnx https://github.com/Xilinx/u-boot-xlnx.git
git fetch xlnx
git checkout master
git checkout -b pcw_merge
git merge --no-edit tags/xilinx-v2019.1  # merge latest changes from xilinx-v2019.1 to current enclustra master
make distclean
