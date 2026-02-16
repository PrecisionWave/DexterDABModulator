
CROSS_PREFIX := riscv-none-elf-
CROSS_CFLAGS := -march=rv32imc
CROSS_AFLAGS := -march=rv32imc

.PHONY : clean all
all : clean hello_a.elf hello_a.asm

%.o: %.S
	$(CROSS_PREFIX)as $(CROSS_AFLAGS) -c $< -o $@

hello_a.elf: entry.o
	$(CROSS_PREFIX)ld -T linker.ld  -o $@ $^

hello_a.asm: hello_a.elf
	$(CROSS_PREFIX)objdump -D -h -r $< > $@

clean:
	-@rm *.o
