#!/bin/bash

ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"

source ${DIR}/settings.sh

# clone analog repository
git clone https://github.com/PrecisionWave/linux.git ${DIR}/${KERNEL_DIR}
git -C ${DIR}/${KERNEL_DIR} checkout ${KERNEL_BRANCH}

# clone enclustra repository
git clone https://github.com/enclustra-bsp/xilinx-linux ${DIR}/enclustra-linux
git -C ${DIR}/enclustra-linux checkout master
