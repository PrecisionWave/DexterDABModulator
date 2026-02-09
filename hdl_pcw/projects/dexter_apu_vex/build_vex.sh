#/bin/bash

pushd VexiiRiscv

if [ ! -e ../Generate.txt ]; then
    sbt "Test/runMain vexiiriscv.Generate --help"
    sbt "Test/runMain vexiiriscv.Generate --help" > ../Generate.txt
fi

sbt "Test/runMain vexiiriscv.Generate \
    --xlen=32 \
    --with-rvm \
    --with-rvc \
    --with-rva \
    --with-rvZb \
    --with-rvZba \
    --with-rvZbb \
    --with-rvZbc \
    --with-rvZbs \
    --with-btb --with-gshare --with-ras \
    --mmu-sync-read \
    --fetch-l1 --fetch-l1-ways=2 \
    --fetch-axi4 \
    --lsu-l1 --lsu-l1-ways=2 --lsu-l1-axi4 \
    --lsu-axi4 \
    --debug-jtag-tap \
    "

cp VexiiRiscv.v ../

popd
