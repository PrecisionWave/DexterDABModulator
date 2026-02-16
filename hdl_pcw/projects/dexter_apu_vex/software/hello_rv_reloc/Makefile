
CROSS_PREFIX := riscv-none-elf-
CROSS_CFLAGS := -march=rv32imc
CROSS_AFLAGS := -march=rv32imc

.PHONY : clean all
all : clean main.elf main.asm

%.o: %.c
	$(CROSS_PREFIX)gcc $(CROSS_CFLAGS) -Wall -c -mcmodel=medany $< -o $@ -ffreestanding

%.o: %.S
	$(CROSS_PREFIX)as $(CROSS_AFLAGS) -c $< -o $@

main.elf: main.o entry.o xuartlite_l.o
	$(CROSS_PREFIX)ld -T linker.ld -nostdlib --emit-reloc -o $@ $^

main.asm: main.elf
	$(CROSS_PREFIX)objdump -D -h -r $< > $@

clean:
	-@rm *.o
