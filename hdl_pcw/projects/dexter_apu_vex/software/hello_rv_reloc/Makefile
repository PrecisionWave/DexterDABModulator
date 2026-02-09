
CROSS_CC := /tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-gcc
CROSS_AS := /tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-as
CROSS_LD := /tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-ld
CROSS_OBJCOPY := /tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-objcopy

.PHONY : clean all
all : clean main.bin

%.o: %.c
	$(CROSS_CC) -Wall -c -mcmodel=medany $< -o $@ -ffreestanding

%.o: %.S
	$(CROSS_AS) -c $< -o $@

main.elf: main.o entry.o xuartlite_l.o
	$(CROSS_LD) -T linker.ld -nostdlib --emit-reloc -o $@ $^

main.bin: main.elf
	$(CROSS_OBJCOPY) -O binary $< $@

clean:
	-@rm *.o


