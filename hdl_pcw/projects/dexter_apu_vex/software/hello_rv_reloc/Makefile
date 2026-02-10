
CROSS_PREFIX := /tools/Xilinx/2025.1/gnu/riscv/lin/riscv64-unknown-elf/bin/riscv64-unknown-elf-

.PHONY : clean all
all : clean main.bin main.asm

%.o: %.c
	$(CROSS_PREFIX)gcc -Wall -c -mcmodel=medany $< -o $@ -ffreestanding

%.o: %.S
	$(CROSS_PREFIX)as -c $< -o $@

main.elf: main.o entry.o xuartlite_l.o
	$(CROSS_PREFIX)ld -T linker.ld -nostdlib --emit-reloc -o $@ $^

main.bin: main.elf
	$(CROSS_PREFIX)objcopy -O binary $< $@

main.asm: main.elf
	$(CROSS_PREFIX)objdump -D -h -r $< > $@

clean:
	-@rm *.o
