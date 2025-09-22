#!/bin/bash

ORG="$(pwd)"
DIR="$ORG/$(dirname $0)"

source ${DIR}/settings.sh

# clone analog repository
git clone git@github.com:PrecisionWave/linux.git ${DIR}/${KERNEL_DIR_ARM}
git -C ${DIR}/${KERNEL_DIR_ARM} checkout ${KERNEL_BRANCH}
# git clone git@github.com:PrecisionWave/linux.git ${DIR}/${KERNEL_DIR_ARM64}
# git -C ${DIR}/${KERNEL_DIR_ARM64} checkout ${KERNEL_BRANCH}

# clone enclustra repository
git clone https://github.com/enclustra-bsp/xilinx-linux ${DIR}/enclustra-linux
git -C ${DIR}/enclustra-linux checkout v1.10  # 2021.2
