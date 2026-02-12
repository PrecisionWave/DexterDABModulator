
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : clean all
all : clean load_apu load_apu_sim

clean:
	-@rm *.o
	-@rm *.oh
	-@rm load_apu
	-@rm load_apu_sim

%.o: %.c
	$(CROSS_CC) -c -Wall -march=armv7 -o $@ $<

load_apu: load_apu.o mmio.o mmap.o memorymap.o load_elf.o load_elf_mb.o load_elf_rv.o
	$(CROSS_CC) -static -o $@ $^


%.oh: %.c
	gcc -c -Wall -DSIM -o $@ $<

load_apu_sim: load_apu.oh mmio.oh mmap.oh memorymap.oh load_elf.oh load_elf_mb.oh load_elf_rv.oh
	gcc -static -DSIM -o $@ $^

test: load_apu_sim load_apu
	-@./load_apu_sim -f main_10k.elf -vvv -x 10k
	/tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-objdump -D -r main_10k.elf > main_10k.asm
	/tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-objcopy main_10k.elf --adjust-vma=0x16800000 -O binary --remove-section vectors.reset main_10k.bin
	/tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-objdump -b binary --adjust-vma 0x16900000 -m riscv -D main_10k.bin > main_10k.adj
	/tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-objdump -b binary --adjust-vma=0x16900000 -m riscv -D 10k.DDR > main_10k.dis
