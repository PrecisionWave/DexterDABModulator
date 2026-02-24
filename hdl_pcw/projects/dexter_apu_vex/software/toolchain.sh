#!/bin/bash

if [ -z "`which xpm`" ]; then
    sudo npm install --location=global xpm@latest
fi

if [ ! -e package.json ]; then
    xpm init
fi

if [ ! -d xpacks ]; then
    xpm install @xpack-dev-tools/riscv-none-elf-gcc@15.2.0-1.1 --verbose
fi

export PATH=`pwd`/xpacks/.bin:$PATH

echo "Done"
